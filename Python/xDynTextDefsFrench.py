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
    "Bidon":        ( "Arial", 12, (1,1,1,1), (0,0,0,0), 0, """Ceci est un objet texte bidon, ne devrait pas apparaître dans le jeu !""", PtJustify.kCenter ),
    "nb01WelcomeToDni":        ( "Sharper", 28, (0,0,0,1), (0,0,0,0), 5, """BIENVENUE À
D'NI""", PtJustify.kCenter ),






    "nb01GoToGrsn":        ( "Sharper", 22, (0,0,0,1), (0,0,0,0), 5, """RENDEZ-VOUS DANS
L'ÂGE GAHREESEN
POUR RÉCUPÉRER
VOTRE KI""", PtJustify.kCenter ),





    "nb01GrsnBook":        ( "Sharper", 22, (0,0,0,1), (0,0,0,0), 5, """LE LIVRE DE LIAISON DE
L'ÂGE GAHREESEN
SE TROUVE DANS LA
SALLE DE LIAISON""", PtJustify.kCenter ),





    "grsnRetrieveKI":        ( "Sharper", 24, (0,0,0,1), (0,0,0,0), 10, """VEUILLEZ
RÉCUPÉRER VOTRE
KI""", PtJustify.kCenter ),





    "nb01EaselWelcome":        ( "Sharper", 28, (0,0,0,1), (0,0,0,0), 5, """
Bienvenue à
BEVIN
Pour plus d'infos,
rendez-vous dans la
salle de classe""", PtJustify.kCenter ),






    "bcoWrinkledNote":        ( "Michelle", 10, (0,0,0,1), (0,0,0,0), 5, """

Dr. Watson -

Gros soucis. La maison de Noloben n'est PAS vide.
J'y ai rencontré quelqu'un aujourd'hui. Je ne parle pas
très bien le D'ni, mais j'ai réussi à communiquer avec
lui un petit moment. Oui, c'est un D'ni et comme nous
nous en doutions, il sait beaucoup de choses sur les
créatures. BEAUCOUP de choses.

Nous devons organiser une réunion dès que possible.

- Marie""", PtJustify.kLeftJustify ),




"WatsonLetter":        ( "Courier", 10, (0,0,0,1), (0,0,0,0), 0, """

Matthew,

Les derniers documents que vous m'avez envoyés étaient très intéressants. Puisque vous avez fait un excellent travail, j'ai une petite liste de tâches à répartir pour l'équipe. Organisez ça comme bon vous semble. 

1. J'aimerais plus d'infos sur les coutumes de la vie en famille : les cérémonies, etc... Tout ce qui se rapporte à la naissance, au mariage et aux événements culturels. Je sais que nous disposons de nombreux documents à étudier, donc récupérez tout ce que vous pouvez. Je pense que nous avons trop axé nos recherches sur les sciences et la technologie et pas assez sur la vie quotidienne de ce peuple.

2. Nous avons beaucoup d'infos sur les Guildes, mais il serait bon d'organiser un peu tout ça.

3. Nous avons de nombreuses lacunes sur la Chute. Je ne suis pas sûr de pouvoir vous aider à ce propos, mais étant données nos dernières infos, je crois qu'il va falloir creuser ce sujet. Je vous recommande de mettre une personne à plein temps sur la Chute.

4. Continuez aussi à travailler sur les Rois. Un résumé de tous les rois serait vraiment le bienvenu.

5. Nous avons encore des documents religieux à faire traduire. Ça risque d'être difficile, mais je crois que ça peut nous apporter de précieuses informations.

6. Nous avons encore un stock de journaux récupérés dans diverses résidences D'ni, etc... sans même parler des Âges.  

Je crois que ça suffira pour l'instant. Merci encore à votre équipe. Dites-leur qu'ils font un excellent boulot. 
 
- Dr. Watson
""", PtJustify.kLeftJustify ),




"JCNote":        ( "Nick", 16, (0,0,0,1), (0,0,0,0), 5, """

Écoutez ça. Je sais que le CRD ne veut pas qu'on y touche, mais je suis sûr que Watson serait ravi de voir le rapport avec les portes. C'est totalement illogique. 

Mais il ne faut pas le perdre. J'ai eu beaucoup de mal à l'enlever du mur, ce qui n'est pas rassurant. Mais le plus étrange, c'est que quand je suis revenu plus tard, l'étoffe était réapparue, intacte.

- Nick
""", PtJustify.kLeftJustify ),






"clftAtrusNote":        ( "Atrus", 18, (0,0,0,1), (0,0,0,0), -5, """

Yeesha chérie,

La nuit dernière, ta mère a fait un rêve...

Le rêve annonce que D 'ni renaîtra un jour. Les
nouveaux explorateurs de D 'ni afflueront dans
le désert, comme irrésistiblement attirés par
quelque chose qu'ils ne comprennent pas.

Le rêve parle aussi d'un oiseau du désert, au
pouvoir capable d'inventer l'avenir de D 'ni.
Nous craignons ce genre de pouvoir - il peut
changer les gens.

Yeesha, notre oiseau du désert, ta quête semble
t'éloigner toujours plus. J'espère que ce que tu
trouveras te rapprochera de nous.

-Atrus

""", PtJustify.kLeftJustify ),






"islmNickNote":        ( "Nick", 16, (0,0,.3,1), (0,0,0,0), 1, """

Où est passé mon Livre ? Pourquoi quelqu'un l'a-t-il pris !

- Nick 

""", PtJustify.kLeftJustify ),
}