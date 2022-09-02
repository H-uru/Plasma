## Resource Manager

### Functional Overview

The Resource Manager processes requests for assets, finds those assets in
memory, disk cache, or from network servers, and notifies the requestor when
that asset is available. It is also responsible for notifying users of an asset
when that asset is no longer available.

### References

The Resource Manager supports two classes of references to other objects.

#### Active References

An Active Reference establishes a dependency, or partial ownership of one object
over another. When an object requests an active reference to another, the
Resource Manager will load the requested object if it isn’t already available in
memory, and then notify the requestor that it is available via a plRefMsg (or
derived class) supplied by the requestor. The Resource Manager also tracks the
active references on an object. If the requesting object is deleted, the number
of active references on the referenced object is decremented. If that number
goes to zero, then the referenced object is also automatically deleted. Note
that requesting an Active Reference to an object will initiate its loading if
the object is not currently in memory, but the load is not guaranteed to be
immediate. Whether loading is asynchronous or single-threaded, there may be some
time before the notification that the requested object is available is
delivered. The object requesting the asset should make provisions for remaining
idle in the interim.

#### Passive References

Passive References establish no dependency. If an object requests a Passive
Reference to another object, the requestor is notified whenever the requested
object comes into memory and also whenever it is deleted. If the requested
object is in memory when the request is made, there is an immediate notification
delivered to the requestor. Otherwise, notifications are delivered only when the
requested object comes and goes from memory. A Passive Reference never causes an
object to be loaded, it only assures that the requestor will be kept informed as
to whether it is loaded or not.

The Passive Reference serves two purposes. First, it allows cyclic references
without cyclic dependencies. If an object makes an Active Reference to another,
it establishes itself as a master of the second object in that the first
object’s deletion may automatically cause the second object’s deletion (if no
other objects have active references to it). The second object may also need a
reference (pointer) to the first object (its “owner”) in order to perform its
function. It may safely request a Passive Reference back to the first object,
because no dependency is formed. If the first object is deleted, but the second
object isn’t (because other objects also have Active References to it), the
second object is notified that its reference (pointer) to the first object is no
longer valid. But the Passive Reference will not interfere with the first
object’s deletion.

The second purpose is in making tentative references to objects. If an object
wants to operate on another object when the object is available, but is content
to continue operation without it when it is not available, a Passive Reference
will keep it informed without forcing the second object into memory.

### Notifications

The Resource Manager notifies the requestors of assets of the availability and
deletion of those assets through user supplied messages, derived from plRefMsg.
The asset requestor fills in a plRefMsg with its return address (plKey), and any
other information it will need to process the asset when it is delivered (if the
actual message is a derived type). When the asset becomes available, the
Resource Manager fills in the reference value (fRef) with a pointer to the
asset, and sets the delivery context. The same message will be used to notify
the requestor when the asset is no longer available, but with a different
context. See plRefMsg below.

### Asset Identification

Assets available from the Resource Manager and objects able to request those
assets are derived from plKeyedObject. The plKey is used as the token
identifying the requested asset to the Resource Manager. The same plKey is also
used by the messaging system to deliver the notification message to the
requesting object.

The plKey is currently uniquely identified by a UOID. The UOID is a combination
of a plLocation (Age,Chapter,Page) and a plObjectID (sequence number, class id,
and copy index). It also contains a scoped string name (pointer into a string
table). The string name is unique within the scope of the object’s Age, Chapter,
Page, and ClassID (plLocation plus type field of plObjectID). The string pointer
on the plKey and the sequence field of the plObjectID form a redundant pair. The
string is human readable and the sequence field is immediately more efficient
for lookup.

The plKey is dereferencable into a pointer to an object, but this mechanism
should be used with great discretion. First, the pointer returned may be nil if
the object isn’t currently loaded. Second, the pointer may become invalid at any
time should the object be paged out, so should never be stored. It is only
marginally safe for immediate use, and becomes even more dangerous in a
multithreaded environment. The only safe mechanism for obtaining a pointer to
another managed object is through the Active and Passive Reference mechanisms
discussed.

### Notification Messages

An object requesting an asset will be notified of that asset’s availability or
deletion via a plRefMsg (or derived class). The plRefMsg defines a set of
notification contexts. Between the context, the supplied pointer to the
reference, and any other information the requestor has packed into its variant
on the plRefMsg, the requestor is expected to react appropriately to any change
in the availability of an asset.

The receiver of a plRefMsg is generally the originator of it, but not always.
plRefMsg’s may also be used by one object to add itself or other 3<sup>rd</sup>
party objects to a target. A common example is the various scene data adding
themselves to paging units (plSceneNode). In the following discussion, the
receiver will be the object receiving the plRefMsg from the ResourceManager. The
asset will refer to the pointer to a C++ object contained in the plRefMsg.

There are two normal sources of plRefMsg’s.

During the receiver’s Read(), it reads the key of an asset with a request for
notification when that asset is available. The receiver supplies a plRefMsg (or
derived type) to the Resource Manager as part of the request. The same plRefMsg
will be used to notify the receiver/requestor when the asset is no longer
available. The interface for this is hsResMgr::ReadKeyNotifyMe().

During an asset’s Read(), it reads the key of an object that it wants to affect,
or that should manage it. The asset sends itself via a plRefMsg to the receiver
object (which should be prepared to receive such a reference). The interface for
this is hsResMgr::AddViaNotify().

In all cases, the Resource Manager is at least an intermediary to the delivery
of a plRefMsg. plRefMsg’s should not be sent directly through the dispatch
system from one object to another.

The defined contexts are as follows:

* ***kOnCreate*** – The asset has just been created or loaded. The receiver is
  free to cache and use its pointer until notified otherwise. This context
  generally means the object has either just been created on scene conversion,
  or has just been loaded into memory. Typically from
  hsResMgr::ReadKeyNotifyMe().
* ***kOnDestroy*** – The asset is about to be destroyed. The receiver should
  discard any references to its pointer value that it may have cached. This
  context usually indicates that the object is being paged out. Typically sent
  automatically as notification when an object is destroyed.
* ***kOnRequest*** – This is a request to add the asset reference, usually to a
  list of like objects. Examples are adding a light to a SceneNode or a Modifier
  to a SceneObject. This context usually implies that the plRefMsg did not
  originate from the receiver. The plRefMsg might have come from the asset
  itself, or from a 3rd party manager object. Typically from
  hsResMgr::AddViaNotify().
* ***kOnRemove*** – The asset is requesting that it be removed, usually from a
  list of like objects. This is typically the reversal of a previous kOnRequest
  message. Typically from one plKey releasing another.
* ***kOnReplace*** – An asset is being delivered as a replacement to another
  asset already received. References to the specified old asset should be
  discarded and replaced by references to the new one. Not currently used.

### Current Limitations

#### Keys for Dynamic Objects

The current system uses UOID’s as the underlying identification method for
plKey’s. The last field of the UOID is a sequence number, which is assigned at
scene conversion time. This leaves no provision for objects created at run-time
(procedural objects). There is a mechanism supplied (plTempKey), but these
aren’t well integrated into the rest of the resource manager. The requirements
for procedural objects are the abilities at run-time to:

* Register with the Resource Manager.
* Receive from the Resource Manager a plKey that other objects will know how to
  request.
* Interact with non-procedural assets normally.
* UnRegister from the Resource Manager at any time.
* If still registered, delete at shutdown via the normal dependency mechanism.

#### Identifiers Across Source Asset Files

Again because the UOID’s are generated at scene conversion time, there is
currently no way for an object in one source Max file to reference an object in
another Max file. The requirement is that the
Age/Chapter/Page/ClassID/&lt;string identifier&gt; of an object may be
efficiently converted at run-time into a plKey for the object. This key can then
be used to deliver mail to the object, or to request a reference to the object
from the Resource Manager. This would preferably be through the same general
mechanism as more efficient normal static references are made and unmade, only
using a more persistent label than the UOID.

#### Garbage Collection

The Resource Manager currently deletes everything still registered in it at
shutdown. This redundant garbage collection masks memory leaks and improper uses
of the reference system. It would be helpful for the Resource Manager to flag
key memory leak objects, but it shouldn’t try to correct for the error. For
example, the Resource Manager can determine that of the six objects remaining
registered at shutdown, five of them would have been automatically deleted if
only the sixth had been released. This is determinable since the Active
References always form a DAG.

#### Asynchronous Loading

A project requirement is to never stall the main thread for loading assets
(although stalling or distracting the user are options). While most of the code
is safe for deferred loading through a separate thread, there appear to be
complication is certain key parts of the system. One requested capability to
make asynchronous loading easier for coders to deal with is a notification
method which reports when an object is not only loaded, but all of its
dependencies are also loaded.

Aside from that extension, moving to asynchronous loading sooner rather than
later might prevent major rewrites of code sections written in a way
incompatible with unpredictable delays between requesting an asset and receiving
it.

#### String-based Key Lookup

String-based key lookup is used in many parts of the engine, but is not really
‘officially’ supported (currently done via StupidSearch()). If you were to
comment out the fIDName field from plKey, the code wouldn’t run currently. If
our goal is to have stringless keys, then we’d better stop depending on ini,
fni, and console commands to init and run the engine, and to collect info from
the user (like avatar and age selection). String usage will continue to grow if
we’re not careful. StupidSearch() has been modified and extended to allow
wild-card location and object search, but since it is not officially supported,
other string lookup mechanisms (like plAvatarMod::Find()) have been created. I
think we need to make a decision one way or the other on this, and if we really
don’t want it, then turn it off in the release build.

#### Robust Cloning

Cloning was added after the fact and is quite fragile. It only works for
avatars, relies on globals, and won’t work asynchronously. Also, it doesn’t
clone the current state, it reloads the avatar under a different uoid. Still,
it’s a useful feature, but probably needs to be re-examined in a future system.

#### Merging Export

Not being able to export from different files into the same page DB is a serious
limitation. It requires every page to be contained in exactly one Max file and
makes exporting multiple avatars difficult.

#### Deterministic UOID Generation

UOID Generation at export seems to rely on the current set of database files in
your export directory to come up with a unique ID. A deterministic solution
would be more desirable. This may have been talked about but just not
implemented.

## Appendix

### Detailed Reference Taxonomy

We currently support four methods of establishing links between objects in
Plasma2.0.

I’m going to try to describe the four methods here so we can better analyze
what functionality can be collapsed, and which methods truly are orthogonal.

For clarity, I’ll use the convention that “you” are the object making a
reference to another object, and “the object” is that object being referenced.

#### Direct Access

The strongest link is directly through a pointer, possibly using a ref-count.
This type of connection implies definite ownership. You keep a pointer to
another object that is a separate C++ object, but logically an extension of you.

One example is the controller a TMModifier uses to do the actual storage and
interpolation of keyframe data. Another is the plMessages, for which the
plDispatch uses hsRefCnt’s to ensure the message isn’t deleted before having
been delivered to all recipients, and to ensure the message is deleted after
being delivered. You may call any functions directly off the object, and are
responsible for its destruction.

#### Active Reference

The second strongest is the active reference. It is different from RefCnt’s in
that it involves the ResourceManager, rather than interfacing directly with the
heap system. This causes two differences in behavior. First, on initiating an
Active Reference, there may be some delay before the object becomes available.
Once it is available, you may interface with the object directly through member
functions.  
You are guaranteed to keep it until you release it. The second difference comes
when everyone has released it. With a RefCnt system, the object is then deleted.

With Active References, the Resource Manager is free to make an intelligent
decision based on available resources and typical behavior. When all Active
References to an object are released, the Resource Manager may decide to delete
the object entirely, delete from memory but cache it on disk, or hold on to it
in memory. Obviously, when making an active ref, you shouldn’t also increment
the object’s hsRefCnt, since the resource manager is handling things at that
level. Note that there is a possibility of dependency loops with active
references, and that these would generally be bad.

#### Passive Reference

One step weaker is the passive reference. The passive reference allows you to be
notified when an object exists so that you may perform actions directly on it,
but doesn’t give you any control over when the object exists. The basic idea is
there are times when you need to do some processing if a target object exists,
but if it doesn’t, you don’t need to do anything. You certainly don’t want to
cause the object to be loaded, you just want to stay informed as to whether it
is currently in the scene or not. This corresponds in Plasma1.0 to getting a
reference key, and checking to see if the ObjectPtr is nil, in which case there
is no work to perform, otherwise do your thing. The passive reference allows the
same behavior, but without polling to see if the object is there each frame.
Since you will be notified when the object is created or destroyed, you can
remain idle until you are notified it has been created, and at that time
register for update messages. When you are notified the object has been
destroyed, you can deregister for updates. This is slightly different (and
preferable) to simply returning after doing nothing if the object isn’t there,
since you won’t even be traversed in that case. Like the active ref, combining a
passive ref with manipulations of the hsRefCnt will only interfere with the
resource manager. Note that dependency loops with passive references (or
combinations of active and passive references) don’t cause any problems.

#### Messages

The weakest, and hopefully most common form of object interaction is through
messages. With messages, the most direct link you have to the object is an
address appropriate for the message dispatch system. You send commands and
requests off to the object, and if the object is there is will perform the
requested action or reply with the requested info. Otherwise, the message is
(optionally) queued for the object and delivered on its creation. Messages also
propagate across the network in an intelligent way, to keep the different
networked incarnations in sync. You never actually have access to a pointer to
the object, but then you never need one either.
