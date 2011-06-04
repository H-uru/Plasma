/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef OBJECT_FLOCKER_H
#define OBJECT_FLOCKER_H

#include "../pnModifier/plSingleModifier.h"

class hsStream;
class hsResMgr;
class plRandom;
class pfObjectFlocker;

// Database tokens for our prox database
template <class T>
class pfTokenForProximityDatabase
{
public:
	virtual ~pfTokenForProximityDatabase() {}

	// call this when your position changes
	virtual void UpdateWithNewPosition(const hsPoint3 &newPos) = 0;

	// find all close-by objects (determined by center and radius)
	virtual void FindNeighbors(const hsPoint3 &center, const float radius, std::vector<T> &results) = 0;
};

// A basic prox database (might need to be optimized in the future)
template <class T>
class pfBasicProximityDatabase
{
public:
	class tokenType;
	typedef std::vector<tokenType*> tokenVector;
	typedef typename tokenVector::const_iterator tokenIterator;

	// "token" to represent objects stored in the database
	class tokenType: public pfTokenForProximityDatabase<T>
	{
	private:
		tokenVector& fTokens;
		T fParent;
		hsPoint3 fPosition;

	public:
		// constructor
		tokenType(T parentObject, tokenVector& tokens) : fParent(parentObject), fTokens(tokens)
		{
			fTokens.push_back(this);
		}

		// destructor
		virtual ~tokenType()
		{
			// remove this token from the database's vector
			fTokens.erase(std::find(fTokens.begin(), fTokens.end(), this));
		}

		// call this when your position changes
		void UpdateWithNewPosition(const hsPoint3 &newPosition) {fPosition = newPosition;}

		// find all close-by objects (determined by center and radius)
		void FindNeighbors(const hsPoint3 &center, const float radius, std::vector<T> & results)
		{
			// take the slow way, loop and check every one
			const float radiusSquared = radius * radius;
			for (tokenIterator i = fTokens.begin(); i != fTokens.end(); i++)
			{
				const hsVector3 offset(&center, &((**i).fPosition));
				const float distanceSquared = offset.MagnitudeSquared();

				// push onto result vector when within given radius
				if (distanceSquared < radiusSquared)
					results.push_back((**i).fParent);
			}
		}
	};

private:
	// STL vector containing all tokens in database
	tokenVector fGroup;

public:
	// constructor
	pfBasicProximityDatabase(void) {}

	// destructor
	virtual ~pfBasicProximityDatabase() {}

	// allocate a token to represent a given client object in this database
	tokenType *MakeToken(T parentObject) {return TRACKED_NEW tokenType(parentObject, fGroup);}

	// return the number of tokens currently in the database
	int Size(void) {return group.size();}
};

// A basic vehicle class that handles accelleration, braking, and turning
class pfVehicle
{
private:
	hsPoint3	fPos; // position in meters
	hsPoint3	fLastPos; // the last position we had
	hsPoint3	fSmoothedPosition;

	hsVector3	fVel; // velocity in meters/second
	hsVector3	fSmoothedAcceleration;

	hsVector3	fForward; // forward vector (unit length)
	hsVector3	fLastForward; // the last forward vector we had
	hsVector3	fSide; // side vector (unit length)
	hsVector3	fUp; // up vector (unit length)

	float		fSpeed; // speed (length of velocity vector)
	float		fMass; // mass of the object (defaults to 1)
	float		fMaxForce; // the maximum steering force that can be applied
	float		fMaxSpeed; // the maximum speed of this vehicle

	float		fCurvature;
	float		fSmoothedCurvature;

	float		fRadius;

	// measure the path curvature (1/turning radius), maintain smoothed version
	void IMeasurePathCurvature(const float elapsedTime);
public:
	pfVehicle() {Reset();}
	virtual ~pfVehicle() {}

	void Reset();

	// get/set attributes
	float Mass() const {return fMass;}
	float SetMass(float m) {return fMass = m;}

	hsVector3 Forward() const {return fForward;}
	hsVector3 SetForward(hsVector3 forward) {return fForward = forward;}
	hsVector3 Side() const {return fSide;}
	hsVector3 SetSide(hsVector3 side) {return fSide = side;}
	hsVector3 Up() const {return fUp;}
	hsVector3 SetUp(hsVector3 up) {return fUp = up;}

	hsPoint3 Position() const {return fPos;}
	hsPoint3 SetPosition(hsPoint3 pos) {return fPos = pos;}

	hsVector3 Velocity() const {return Forward() * fSpeed;}

	float Speed() const {return fSpeed;}
	float SetSpeed(float speed) {return fSpeed = speed;}

	float MaxForce() const {return fMaxForce;}
	float SetMaxForce(float maxForce) {return fMaxForce = maxForce;}

	float MaxSpeed() const {return fMaxSpeed;}
	float SetMaxSpeed(float maxSpeed) {return fMaxSpeed = maxSpeed;}

	float Curvature() const {return fCurvature;}

	float SmoothedCurvature() {return fSmoothedCurvature;}
	float ResetSmoothedCurvature(float value = 0);

	hsVector3 SmoothedAcceleration() {return fSmoothedAcceleration;}
	hsVector3 ResetSmoothedAcceleration(const hsVector3 &value = hsVector3(0,0,0));

	hsPoint3 SmoothedPosition() {return fSmoothedPosition;}
	hsPoint3 ResetSmoothedPosition(const hsPoint3 &value = hsPoint3(0,0,0));

	float Radius() const {return fRadius;}
	float SetRadius(float radius) {return fRadius = radius;}

	// Basic geometry functions

	// Reset local space to identity
	void ResetLocalSpace();

	// Set the side vector to a normalized cross product of forward and up
	void SetUnitSideFromForwardAndUp();

	// Regenerate orthonormal basis vectors given a new forward vector (unit length)
	void RegenerateOrthonormalBasisUF(const hsVector3 &newUnitForward);

	// If the new forward is NOT known to have unit length
	void RegenerateOrthonormalBasis(const hsVector3 &newForward)
	{hsVector3 temp = newForward; temp.Normalize(); RegenerateOrthonormalBasisUF(temp);}

	// For supplying both a new forward, and a new up
	void RegenerateOrthonormalBasis(const hsVector3 &newForward, const hsVector3 &newUp)
	{fUp = newUp; RegenerateOrthonormalBasis(newForward);}

	// Keep forward parallel to velocity, change up as little as possible
	virtual void RegenerateLocalSpace(const hsVector3 &newVelocity, const float elapsedTime);

	// Keep forward parallel to velocity, but "bank" the up vector
	void RegenerateLocalSpaceForBanking(const hsVector3 &newVelocity, const float elapsedTime);

	// Vehicle physics functions

	// apply a steering force to our momentum and adjust our
	// orientation to match our velocity vector
	void ApplySteeringForce(const hsVector3 &force, const float deltaTime);

	// adjust the steering force passed to ApplySteeringForce (so sub-classes can refine)
	// by default, we won't allow backward-facing steering at a low speed
	virtual hsVector3 AdjustRawSteeringForce(const hsVector3 &force, const float deltaTime);

	// apply a braking force
	void ApplyBrakingForce(const float rate, const float deltaTime);

	// predict the position of the vehicle (assumes constant velocity)
	hsPoint3 PredictFuturePosition(const float predictionTime);
};

// A goal object, basically keeps track of a scene object so we can get velocity from it
class pfBoidGoal
{
private:
	hsPoint3	fLastPos;
	hsPoint3	fCurPos;
	hsVector3	fForward;
	float		fSpeed; // in meters/sec
	hsBool		fHasLastPos; // does the last position make sense?
public:
	pfBoidGoal();
	~pfBoidGoal() {}

	void Update(plSceneObject *goal, float deltaTime);

	hsPoint3 Position() const {return fCurPos;}
	float Speed() const {return fSpeed;}
	hsVector3 Forward() const {return fForward;}
	hsPoint3 PredictFuturePosition(const float predictionTime);
};

typedef pfTokenForProximityDatabase<pfVehicle*> pfProximityToken;
typedef pfBasicProximityDatabase<pfVehicle*> pfProximityDatabase;

// The actual "flocking following" (not really a boid, but whatever)
class pfBoid: public pfVehicle
{
private:
	plKey		fObjKey;

	float		fWanderSide;
	float		fWanderUp;

	float		fGoalWeight;
	float		fRandomWeight;

	float		fSeparationRadius;
	float		fSeparationAngle;
	float		fSeparationWeight;

	float		fCohesionRadius;
	float		fCohesionAngle;
	float		fCohesionWeight;

	pfProximityToken* fProximityToken;
	std::vector<pfVehicle*> fNeighbors;

	// Set our flocking settings to default
	void IFlockDefaults();

	// Setup our prox database token
	void ISetupToken(pfProximityDatabase &pd);

	// Are we in the neighborhood of another boid?
	hsBool IInBoidNeighborhood(const pfVehicle &other, const float minDistance, const float maxDistance, const float cosMaxAngle);
	// Wander steering
	hsVector3 ISteerForWander(float timeDelta);
	// Seek the target point
	hsVector3 ISteerForSeek(const hsPoint3 &target);
	// Steer the boid toward our goal
	hsVector3 ISteerToGoal(pfBoidGoal &goal, float maxPredictionTime);
	// Steer to keep separation
	hsVector3 ISteerForSeparation(const float maxDistance, const float cosMaxAngle, const std::vector<pfVehicle*> &flock);
	// Steer to keep the flock together
	hsVector3 ISteerForCohesion(const float maxDistance, const float cosMaxAngle, const std::vector<pfVehicle*> &flock);

public:
	pfObjectFlocker *fFlockerPtr;

	pfBoid(pfProximityDatabase &pd, pfObjectFlocker *flocker, plKey &key);
	pfBoid(pfProximityDatabase &pd, pfObjectFlocker *flocker, plKey &key, hsPoint3 &pos);
	pfBoid(pfProximityDatabase &pd, pfObjectFlocker *flocker, plKey &key, hsPoint3 &pos, float speed, hsVector3 &forward, hsVector3 &side, hsVector3 &up);
	virtual ~pfBoid();

	// Get/set functions
	float GoalWeight() const {return fGoalWeight;}
	float SetGoalWeight(float goalWeight) {return fGoalWeight = goalWeight;}
	float WanderWeight() const {return fRandomWeight;}
	float SetWanderWeight(float wanderWeight) {return fRandomWeight = wanderWeight;}

	float SeparationWeight() const {return fSeparationWeight;}
	float SetSeparationWeight(float weight) {return fSeparationWeight = weight;}
	float SeparationRadius() const {return fSeparationRadius;}
	float SetSeparationRadius(float radius) {return fSeparationRadius = radius;}

	float CohesionWeight() const {return fCohesionWeight;}
	float SetCohesionWeight(float weight) {return fCohesionWeight = weight;}
	float CohesionRadius() const {return fCohesionRadius;}
	float SetCohesionRadius(float radius) {return fCohesionRadius = radius;}

	// Update the boid's data based on the goal and time delta
	void Update(pfBoidGoal &goal, float deltaTime);
	plKey &GetKey() {return fObjKey;}

	// We're redirecting this to the "banking" function
	virtual void RegenerateLocalSpace(const hsVector3 &newVelocity, const float elapsedTime);
};

class pfFlock
{
private:
	std::vector<pfBoid*>	fBoids;
	pfBoidGoal				fBoidGoal;
	pfProximityDatabase		*fDatabase;

	// global values so when we add a boid we can set it's parameters
	float fGoalWeight, fRandomWeight;
	float fSeparationWeight, fSeparationRadius;
	float fCohesionWeight, fCohesionRadius;
	float fMaxForce; // max steering force
	float fMaxSpeed, fMinSpeed;
public:
	pfFlock();
	~pfFlock();

	// Get/set functions (affect the whole flock, and any new boids added)
	float GoalWeight() const {return fGoalWeight;}
	void SetGoalWeight(float goalWeight);
	float WanderWeight() const {return fRandomWeight;}
	void SetWanderWeight(float wanderWeight);

	float SeparationWeight() const {return fSeparationWeight;}
	void SetSeparationWeight(float weight);
	float SeparationRadius() const {return fSeparationRadius;}
	void SetSeparationRadius(float radius);

	float CohesionWeight() const {return fCohesionWeight;}
	void SetCohesionWeight(float weight);
	float CohesionRadius() const {return fCohesionRadius;}
	void SetCohesionRadius(float radius);

	float MaxForce() const {return fMaxForce;}
	void SetMaxForce(float force);
	float MaxSpeed() const {return fMaxSpeed;}
	void SetMaxSpeed(float speed);
	float MinSpeed() const {return fMinSpeed;}
	void SetMinSpeed(float minSpeed);

	// setup/run functions
	void AddBoid(pfObjectFlocker *flocker, plKey &key, hsPoint3 &pos);
	void Update(plSceneObject *goal, float deltaTime);
	pfBoid *GetBoid(int i);

	friend class pfObjectFlocker;
};

class pfObjectFlocker : public plSingleModifier
{
public:
	pfObjectFlocker();
	~pfObjectFlocker();

	CLASSNAME_REGISTER( pfObjectFlocker );
	GETINTERFACE_ANY( pfObjectFlocker, plSingleModifier );

	virtual void SetTarget(plSceneObject* so);
	virtual hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	void SetNumBoids(UInt8 val);
	void SetBoidKey(plKey key) { fBoidKey = key; }

	float GoalWeight() const {return fFlock.GoalWeight();}
	void SetGoalWeight(float goalWeight) {fFlock.SetGoalWeight(goalWeight);}
	float WanderWeight() const {return fFlock.WanderWeight();}
	void SetWanderWeight(float wanderWeight) {fFlock.SetWanderWeight(wanderWeight);}

	float SeparationWeight() const {return fFlock.SeparationWeight();}
	void SetSeparationWeight(float weight) {fFlock.SetSeparationWeight(weight);}
	float SeparationRadius() const {return fFlock.SeparationRadius();}
	void SetSeparationRadius(float radius) {fFlock.SetSeparationRadius(radius);}

	float CohesionWeight() const {return fFlock.CohesionWeight();}
	void SetCohesionWeight(float weight) {fFlock.SetCohesionWeight(weight);}
	float CohesionRadius() const {return fFlock.CohesionRadius();}
	void SetCohesionRadius(float radius) {fFlock.SetCohesionRadius(radius);}

	float MaxForce() const {return fFlock.MaxForce();}
	void SetMaxForce(float force) {fFlock.SetMaxForce(force);}
	float MaxSpeed() const {return fFlock.MaxSpeed();}
	void SetMaxSpeed(float speed) {fFlock.SetMaxSpeed(speed);}
	float MinSpeed() const {return fFlock.MinSpeed();}
	void SetMinSpeed(float minSpeed) {fFlock.SetMinSpeed(minSpeed);}

	hsBool RandomizeAnimStart() const {return fRandomizeAnimationStart;}
	void SetRandomizeAnimStart(hsBool val) {fRandomizeAnimationStart = val;}
	hsBool UseTargetRotation() const {return fUseTargetRotation;}
	void SetUseTargetRotation(hsBool val) {fUseTargetRotation = val;}

protected:
	const static int fFileVersion; // so we don't have to update the global version number when we change

	pfFlock fFlock;
	int fNumBoids;
	plKey fBoidKey;

	hsBool fUseTargetRotation;
	hsBool fRandomizeAnimationStart;

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty);
};

#endif
