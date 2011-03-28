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
    "nb01WelcomeToDni":        ( "Sharper", 28, (0,0,0,1), (0,0,0,0), 5, """WILLKOMMEN
IN D'NI""", PtJustify.kCenter ),






    "nb01GoToGrsn":        ( "Sharper", 22, (0,0,0,1), (0,0,0,0), 5, """BITTE HOLEN SIE
SICH IHRE KI IN
DER WELT
GAHREESEN""", PtJustify.kCenter ),





    "nb01GrsnBook":        ( "Sharper", 22, (0,0,0,1), (0,0,0,0), 0, """DAS BUCH FÜR
GAHREESEN
BEFINDET SICH
IM RAUM DER
VERBINDUNGEN""", PtJustify.kCenter ),





    "grsnRetrieveKI":        ( "Sharper", 24, (0,0,0,1), (0,0,0,0), 10, """BITTE
HOLEN SIE SICH
IHRE KI""", PtJustify.kCenter ),





    "nb01EaselWelcome":        ( "Sharper", 28, (0,0,0,1), (0,0,0,0), 5, """
Willkommen in
BEVIN
Weitere Informationen
erhalten Sie
im Unterrichtsraum""", PtJustify.kCenter ),






    "bcoWrinkledNote":        ( "Michelle", 10, (0,0,0,1), (0,0,0,0), 5, """

Dr. Watson -

Es gibt Probleme. Das Haus in Noloben ist NICHT leer.
Ich habe heute dort jemanden angetroffen. Mein D'ni
ist nicht besonders, aber ich habe mich eine Weile
mit ihm unterhalten. Er sagt er sei ein D'ni und wie
wir bereits angenommen haben, weiß er eine Menge
über diese Kreaturen.
WIRKLICH EINE GANZE Menge.

Wir sollten umgehend ein Meeting einberufen.

- Marie""", PtJustify.kLeftJustify ),




"WatsonLetter":        ( "Courier", 10, (0,0,0,1), (0,0,0,0), 0, """

Matthew,

Der letzte Satz Unterlagen, den du geschickt hast, war sehr interessant. Ich habe noch eine weitere Liste, von der ich möchte, dass du sie an das Team weiter gibst. Wie du sie genau aufteilst, ist deine Sache. 

1. Ich möchte mehr Informationen über das Familienleben: Zeremonien, etc... Einfach alles, was mit Geburt, Heirat, kulturellen Ereignissen zusammenhängt. Ich weiß, dass wir einige Quellen haben, daher wäre deine Hilfe sehr wertvoll für mich. Über Wissenschaft und Technologie haben wir ja schon einiges zusammengetragen, aber ich finde, dass wir noch nicht genug über das Leben dieses Volkes wissen.

2. Wir haben einiges an Informationen über die Gilden gesammelt. Eine ordentliche Aufstellung an einem Ort wäre sehr praktisch.

3. Der Fall ist etwas, über das wir nur sehr wenig wissen. Ich weiß nicht, ob du mir da weiterhelfen kannst, aber nach den Nachrichten der letzten Zeit, sollten wir uns das mal näher ansehen. Ich würde empfehlen, jemanden ausschließlich auf den Fall anzusetzen.

4. Macht mit den Königen weiter. Ein kurzer Abriss zu allen Königen entsprechend denen im letzten Satz wäre hilfreich.

5. Wir haben immer noch religiöse Schriften, die übersetzt werden müssen. Das wird nicht einfach werden, aber ich bin sicher, dass wir darin sehr wertvolle Informationen finden werden.

6. Wir haben einen ganzen Stapel Tagebücher aus den verschiedenen D'ni-Unterkünften, etc. - von Welten gar nicht zu reden ...  

Ich glaube, das ist mehr als genug. Noch mal vielen Dank - auch an dein Team, das großartige Arbeit macht. 
- Dr. Watson
""", PtJustify.kLeftJustify ),




"JCNote":        ( "Nick", 16, (0,0,0,1), (0,0,0,0), 0, """

Sieh dir das mal an. Ich weiß, dass der DRC nicht, will, dass wir die Dinger anfassen, aber ich wette, Watson will auch wissen, wie die mit den Türen in Verbindung stehen. Es macht einfach keinen Sinn. 

Und verlier es nicht. Ich habe es kaum von der Wand bekommen und als ich es endlich geschafft hatte, war es ganz schön unheimlich. Aber etwas ist noch seltsamer: Als ich später zurückkam, war das Tuch, von dem ich dieses Stück habe, wieder intakt.

- Nick
""", PtJustify.kLeftJustify ),






"clftAtrusNote":        ( "Atrus", 16, (0,0,0,1), (0,0,0,0), -5, """


Yeesha,

in der letzten Nacht hatte deine Mutter einen Traum.

Der Traum kündete davon, dass D'ni eines Tages
wieder auferstehen wird. Sucher der D'ni werden von
der Wüste her kommen werden - gerufen von etwas, was
sie nicht verstehen.

Der Traum handelte auch von einem Wüstenvogel mit
der Macht, diese neue Zukunfts D'nis zu weben.

Wir fürchten eine solche Macht - sie verändert die 
Menschen.

Yeesha, du bist unser Wüstenvogel. Auf deiner Suche
scheinst du dich mehr und mehr von uns zu entfernen.
Ich hoffe, dass das, was du finden wirst, dich uns 
wieder näher bringt.

-Atrus
""", PtJustify.kLeftJustify ),






"islmNickNote":        ( "Nick", 16, (0,0,.3,1), (0,0,0,0), 1, """

Wo zur Hölle ist mein Buch? Wer hat überhaupt etwas da dran zu suchen?!

- Nick 

""", PtJustify.kLeftJustify ),
}
