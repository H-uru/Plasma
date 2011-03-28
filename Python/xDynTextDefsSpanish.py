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
    "Dummy":        ( "Arial", 12, (1,1,1,1), (0,0,0,0), 0, """Este objeto es un texto tonto, ¡no me verás nunca en el juego!""", PtJustify.kCenter ),
    "nb01WelcomeToDni":        ( "Sharper", 28, (0,0,0,1), (0,0,0,0), 5, """BIENVENIDO A 
D'NI""", PtJustify.kCenter ),






    "nb01GoToGrsn":        ( "Sharper", 22, (0,0,0,1), (0,0,0,0), 5, """POR FAVOR, CONECTA
CON LA ERA
GAHREESEN PARA
CONSEGUIR TU KI""", PtJustify.kCenter ),





    "nb01GrsnBook":        ( "Sharper", 22, (0,0,0,1), (0,0,0,0), 5, """EL LIBRO DE LA ERA
GAHREESEN ESTÁ
EL LA SALA DE
CONECCIÓN""", PtJustify.kCenter ),





    "grsnRetrieveKI":        ( "Sharper", 24, (0,0,0,1), (0,0,0,0), 10, """POR FAVOR, 
RECUPERA TU
KI""", PtJustify.kCenter ),





    "nb01EaselWelcome":        ( "Sharper", 28, (0,0,0,1), (0,0,0,0), 5, """
Bienvenido a
BEVIN
Par más información
Ve al 
Aula""", PtJustify.kCenter ),






    "bcoWrinkledNote":        ( "Michelle", 10, (0,0,0,1), (0,0,0,0), 5, """

Dr. Watson -

Tenemos problemas. La casa de Noloben NO está vacía.
Hoy me encontré con alguien allí. Aunque mi D'ni no es
muy bueno hablé un rato con él. Sí, es un D'ni y, tal
y como nos imaginamos, sabe mucho sobre las criaturas.
MUCHÍSIMO.

Obviamente necesitamos convocar una reunión
CUANTO ANTES.

- Marie""", PtJustify.kLeftJustify ),




"WatsonLetter":        ( "Courier", 10, (0,0,0,1), (0,0,0,0), 0, """

Matthew,

El último lote de papeles que enviaste fue muy interesantes. Como has hecho un excelente trabajo, tengo otra lista que me gustaría que dividieras con el equipo. Dependerá de ti el cómo lo hagas. 

1. Me gustaría tener más información sobe la vida en familia, ceremonias, etc... y cualquier cosa relacionada con los nacimientos, las bodas, los eventos culturales. Sé que disponemos de bastante material de referencia al respecto, así que cualquier información adicional que consigas será muy útil. Creo que hemos recopilado abundante material sobre ciencia y tecnología, y en cambio bastante poco sobre la vida personal de esta gente. 

2. Disponemos de bastante información sobre los Gremios y estaría bien hacer una recopilación ordenada de todo ello. 

3. La Caída es claramente un tema en el que flaqueamos. No estoy seguro de poder ayudarte con material de investigación, aunque dada la información que hemos obtenido recientemente, en algún momento lo trataremos más a fondo. Recomiendo que este trabajo se le asigne a alguien en exclusiva.

4. Continúa con los Reyes. Una breve sinopsis de todos los reyes sería de gran ayuda, siguiendo el procedimiento que empezaste con el último lote. 

5. Todavía quedan muchos escritos religiosos por traducir. Van a ser los más difíciles aunque creo que nos podrán aportar mucha información y muy valiosa. 

6. Tenemos un montón de diarios de distintas residencias D'ni, y sitios así... sin contar con las Eras.  

Creo que con esto tendrás más que suficiente. Nuevamente, da las gracias a tu equipo y diles que están haciendo un trabajo excelente. 
- Dr. Watson
""", PtJustify.kLeftJustify ),




"JCNote":        ( "Nick", 16, (0,0,0,1), (0,0,0,0), 5, """

Examina esto. Sé que el DRC no quiere que lo toquemos, aunque apuesto que a Watson también le gustaría saber cómo se regulan las puertas. No tiene sentido. 

Y no lo pierdas. Me costó mucho retirarlo de la pared y cuando lo hice, me dio bastante miedo, quizás lo más extraño fue que cuando regresé después, la tela de la que tomé esta pieza estaba intacta. 

- Nick
""", PtJustify.kLeftJustify ),






"clftAtrusNote":        ( "Atrus", 16, (0,0,0,1), (0,0,0,0), -5, """


Queridísima Yeesha,

Anoche tu madre tuvo un sueño...

Sabemos que algunos futuros no fueron designados
por el autor o por el Hacedor, aunque el sueño
cuenta que D' ni volverá a desarrollarse algún
día. Nuevos exploradores de D' ni llegarán del
desierto, sintiendo la llamada de algo que aún
no comprenden. 

El sueño también habla de un pájaro del desierto
con poderes para tejer ese nuevo destino de
D' ni. Tememos que tal poder cambie a la gente.

Yeesha, nuestro pájaro del desierto, tu búsqueda
parece alejarte más y más de nosotros. Espero
que lo que encuentres te vuelva a acercar a
nosotros. 

-Atrus

""", PtJustify.kLeftJustify ),






"islmNickNote":        ( "Nick", 16, (0,0,.3,1), (0,0,0,0), 1, """

¿Dónde demonios está mi libro? ¡¿Y por qué se lo tuvo que llevar alguien?!

- Nick 

""", PtJustify.kLeftJustify ),
}
