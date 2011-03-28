""" *==LICENSE==*

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

 *==LICENSE==* """
from PlasmaTypes import *
#   "Text Name": ( font name, font size, color, margin, line spacing, text, justification )
# font color is in format (red,green,blue,alpha) with the values between 0 and 1
# text margin is in format (top,left,bottom,right) with the values being in pixels
# line spacing is in pixels and can be positive or negative
# justification is optional, but can be any of the following: PtJustify.kCenter, PtJustify.kLeftJustify, PtJustify.kRightJustify
xTextObjects = {\
    "Dummy":        ( "Arial", 12, (1,1,1,1), (0,0,0,0), 0, """This is a dummy text object, you should never see me in game!""", PtJustify.kCenter ),
    "nb01WelcomeToDni":        ( "Sharper", 28, (0,0,0,1), (0,0,0,0), 5, """WELCOME TO
D'NI""", PtJustify.kCenter ),






    "nb01GoToGrsn":        ( "Sharper", 22, (0,0,0,1), (0,0,0,0), 5, """PLEASE LINK TO
THE GAHREESEN
AGE TO PICK UP
YOUR KI""", PtJustify.kCenter ),





    "nb01GrsnBook":        ( "Sharper", 22, (0,0,0,1), (0,0,0,0), 5, """GAHREESEN AGE
BOOK IS IN
THE LINKING
ROOM""", PtJustify.kCenter ),





    "grsnRetrieveKI":        ( "Sharper", 24, (0,0,0,1), (0,0,0,0), 10, """PLEASE
RETRIEVE YOUR
KI""", PtJustify.kCenter ),





    "nb01EaselWelcome":        ( "Sharper", 28, (0,0,0,1), (0,0,0,0), 5, """
Welcome to
BEVIN
For more Info
Go to the
Classroom""", PtJustify.kCenter ),






    "bcoWrinkledNote":        ( "Michelle", 10, (0,0,0,1), (0,0,0,0), 5, """

Dr. Watson -

Big problems. The house on Noloben is NOT empty.
I met someone there today. My D'ni isn't great, but
I spoke with him for a while. Yeah, he's D'ni and,
as we figured, he knows a lot about the creatures.
A WHOLE lot.

We obviously need a meeting ASAP.

- Marie""", PtJustify.kLeftJustify ),




"WatsonLetter":        ( "Courier", 10, (0,0,0,1), (0,0,0,0), 0, """

Matthew,

The last batch of papers you sent were very interesting. Since you did such a good job, I've got another list I'd like you to divvy up to the team. How you do it is up to you. 

1. I'd like some more information on family life: ceremonies, etc... Anything related to birth, marriage, cultural events. I know we have quite a bit of source material for this so anything you get would be helpful. I think we've gathered quite a bit on science and technology and not enough on the personal lives of these people.

2. We have quite a bit of Guild information but gathering that all up into one tidy area would be nice.

3. The Fall is still an obvious area where we are lacking. I'm not sure I can help you with research material but given the latest information we are getting, at some time, we are going to have to dig into this. I recommend assigning someone the sole task of The Fall.

4. Continue on with the Kings. A short synopsis of all the kings would be helpful following the form you started with the last batch.

5. We still have religious writings, we need to translate. These are going to be the most difficult but I think they can give us large amounts of helpful information.

6. We have a stack of journals from various D'ni residences, etc...not to mention Ages.  

I think that will be more than enough for now. Again, thank your team and tell them they are doing great work. 
 
- Dr. Watson
""", PtJustify.kLeftJustify ),




"JCNote":        ( "Nick", 16, (0,0,0,1), (0,0,0,0), 5, """

Check this out. I know the DRC doesn't want us to touch these, but I bet Watson would like to know how these register with the doors too. It makes no sense. 

And don't lose it. I could barely get it off the wall and when I did, it was pretty scary. Maybe the weirdest thing is that when I went back later, the cloth I got this piece from was intact again.

- Nick
""", PtJustify.kLeftJustify ),






"clftAtrusNote":        ( "Atrus", 16, (0,0,0,1), (0,0,0,0), 0, """

Our dearest Yeesha,

Last night your mother had a dream...

We know that some futures are not cast, by writer
or Maker, but the dream tells that D 'ni will grow
again someday. New seekers of D 'ni will flow
in from the desert, feeling called to something
they do not understand.

But the dream also tells of a desert bird with
the power to weave this new D 'ni's future.
We fear such power - it changes people.

Yeesha, our desert bird, your search seems to
take you further and further from us. I hope that
what you find will bring you closer.

-Your father, Atrus

""", PtJustify.kLeftJustify ),






"islmNickNote":        ( "Nick", 18, (0,0,.3,1), (0,0,0,0), 1, """Where the heck is my book? And why did someone take it in the first place!

- Nick 

""", PtJustify.kLeftJustify ),
}