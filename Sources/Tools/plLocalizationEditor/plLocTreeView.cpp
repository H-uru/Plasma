/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "plLocTreeView.h"
#include "plEditDlg.h"

#include <vector>

#include "pfLocalizationMgr/pfLocalizationDataMgr.h"

void plLocTreeView::LoadData(const ST::string &selectionPath)
{
    ST::string targetAge, targetSet, targetElement, targetLang;
    SplitLocalizationPath(selectionPath, targetAge, targetSet, targetElement, targetLang);
    bool ageMatched = false;
    bool setMatched = false;
    bool elementMatched = false;

    std::vector<ST::string> ages = pfLocalizationDataMgr::Instance().GetAgeList();
    for (int curAge = 0; curAge < ages.size(); curAge++)
    {
        // add the age to the tree
        QTreeWidgetItem *ageItem = new QTreeWidgetItem(this, QStringList { ages[curAge].c_str() });
        ageItem->setData(0, kLocPathRole, QString(ages[curAge].c_str()));

        if (ages[curAge] == targetAge)
        {
            setCurrentItem(ageItem);
            scrollToItem(ageItem);
            ageMatched = true;
        }
        else
            ageMatched = false;

        std::vector<ST::string> sets = pfLocalizationDataMgr::Instance().GetSetList(ages[curAge]);
        for (int curSet = 0; curSet < sets.size(); curSet++)
        {
            std::vector<ST::string> elements = pfLocalizationDataMgr::Instance().GetElementList(ages[curAge], sets[curSet]);

            QTreeWidgetItem *setItem = new QTreeWidgetItem(ageItem, QStringList { sets[curSet].c_str() });
            setItem->setData(0, kLocPathRole, QString("%1.%2").arg(ages[curAge].c_str()).arg(sets[curSet].c_str()));

            if ((sets[curSet] == targetSet) && ageMatched)
            {
                setCurrentItem(setItem);
                scrollToItem(setItem);
                setMatched = true;
            }
            else
                setMatched = false;

            for (int curElement = 0; curElement < elements.size(); curElement++)
            {
                QTreeWidgetItem *subItem = new QTreeWidgetItem(setItem, QStringList { elements[curElement].c_str() });
                subItem->setData(0, kLocPathRole, QString("%1.%2.%3").arg(ages[curAge].c_str())
                                    .arg(sets[curSet].c_str()).arg(elements[curElement].c_str()));

                if (elements[curElement] == targetElement && setMatched)
                {
                    setCurrentItem(subItem);
                    scrollToItem(subItem);
                    elementMatched = true;

                    if (targetLang.empty())
                        targetLang = ST_LITERAL("English");
                }
                else
                    elementMatched = false;

                std::vector<ST::string> languages = pfLocalizationDataMgr::Instance().GetLanguages(ages[curAge], sets[curSet], elements[curElement]);
                for (int curLang = 0; curLang < languages.size(); curLang++)
                {
                    QTreeWidgetItem *langItem = new QTreeWidgetItem(subItem, QStringList { languages[curLang].c_str() });
                    langItem->setData(0, kLocPathRole, QString("%1.%2.%3.%4").arg(ages[curAge].c_str())
                                        .arg(sets[curSet].c_str()).arg(elements[curElement].c_str())
                                        .arg(languages[curLang].c_str()));

                    if (languages[curLang] == targetLang && elementMatched)
                    {
                        setCurrentItem(langItem);
                        scrollToItem(langItem);
                    }
                }
            }
        }
    }

    sortByColumn(0, Qt::AscendingOrder);
}

ST::string plLocTreeView::CurrentPath() const
{
    return (currentItem() != nullptr)
        ? ST::string(currentItem()->data(0, kLocPathRole).toString().toUtf8().constData())
        : ST::string();
}
