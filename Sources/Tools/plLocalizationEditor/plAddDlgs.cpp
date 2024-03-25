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
// basic classes for encapsulating the add dialogs

#include "plAddDlgs.h"
#include "plEditDlg.h"

#include "plResMgr/plLocalization.h"
#include "pfLocalizationMgr/pfLocalizationDataMgr.h"

#include <QPushButton>
#include "res/ui_AddElement.h"
#include "res/ui_AddLocalization.h"

#include <algorithm>
#include <vector>

// very simple validator for edit controls (and combo boxes) so that they only accept alphanumeric values
class AlphaNumericValidator : public QValidator
{
public:
    AlphaNumericValidator(QObject *parent = nullptr) : QValidator(parent) { }

    State validate(QString &input, int &pos) const override
    {
        for (int ch = 0; ch < input.size(); ++ch)
        {
            ushort theChar = input[ch].unicode();
            if ((theChar < '0' || theChar > '9') && (theChar < 'a' || theChar > 'z')
                    && (theChar < 'A' || theChar >'Z'))
                return Invalid;
        }
        return Acceptable;
    }
};

plAddElementDlg::plAddElementDlg(const ST::string &parentPath, QWidget *parent)
    : QDialog(parent), fBlockUpdates(false)
{
    fUI = new Ui_AddElement;
    fUI->setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    AlphaNumericValidator *validator = new AlphaNumericValidator(this);
    fUI->fParentAge->setValidator(validator);
    fUI->fParentSet->setValidator(validator);
    fUI->fElementName->setValidator(validator);

    connect(fUI->fParentAge, SIGNAL(currentTextChanged(QString)), SLOT(Update(QString)));
    connect(fUI->fParentSet, SIGNAL(currentTextChanged(QString)), SLOT(Update(QString)));
    connect(fUI->fElementName, SIGNAL(textChanged(QString)), SLOT(Update(QString)));

    // throw away vars
    ST::string element, lang;
    SplitLocalizationPath(parentPath, fAgeName, fSetName, element, lang);
}

plAddElementDlg::~plAddElementDlg()
{
    delete fUI;
}

bool plAddElementDlg::DoPick()
{
    std::vector<ST::string> ageNames = pfLocalizationDataMgr::Instance().GetAgeList();

    fBlockUpdates = true;
    // add the age names to the list
    for (int i = 0; i < ageNames.size(); i++)
        fUI->fParentAge->addItem(ageNames[i].c_str());

    // select the age we were given
    fUI->fParentAge->setCurrentText("");
    fBlockUpdates = false;
    fUI->fParentAge->setCurrentText(fAgeName.c_str());

    return exec() == QDialog::Accepted;
}

void plAddElementDlg::Update(const QString &text)
{
    if (fBlockUpdates)
        return;

    if (sender() == fUI->fParentAge)
        fAgeName = ST::string(text.toUtf8().constData());
    else if (sender() == fUI->fParentSet)
        fSetName = ST::string(text.toUtf8().constData());
    else if (sender() == fUI->fElementName)
        fElementName = ST::string(text.toUtf8().constData());

    fUI->fPathLabel->setText(tr("%1.%2.%3").arg(fAgeName.c_str())
                             .arg(fSetName.c_str()).arg(fElementName.c_str()));

    if (sender() == fUI->fParentAge) // we only update this if the age changed
    {
        // now add the sets
        fUI->fParentSet->clear();
        fUI->fParentSet->clearEditText();

        std::vector<ST::string> setNames = pfLocalizationDataMgr::Instance().GetSetList(fAgeName);

        // add the set names to the list
        fBlockUpdates = true;
        for (int i = 0; i < setNames.size(); i++)
            fUI->fParentSet->addItem(setNames[i].c_str());

        // select the set we currently have
        fUI->fParentSet->setCurrentText("");
        fBlockUpdates = false;
        fUI->fParentSet->setCurrentText(fSetName.c_str());
    }

    bool valid = !(fAgeName.empty() || fSetName.empty() || fElementName.empty());
    fUI->fButtons->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

// plAddLocalizationDlg - dialog for adding a single localization
plAddLocalizationDlg::plAddLocalizationDlg(const ST::string &parentPath, QWidget *parent)
    : QDialog(parent)
{
    fUI = new Ui_AddLocalization;
    fUI->setupUi(this);
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    connect(fUI->fLanguage, SIGNAL(currentIndexChanged(int)), SLOT(SelectLanguage(int)));

    // throw away vars
    ST::string lang;
    SplitLocalizationPath(parentPath, fAgeName, fSetName, fElementName, lang);
}

bool plAddLocalizationDlg::DoPick()
{
    fUI->fPathLabel->setText(tr("%1.%2.%3").arg(fAgeName.c_str())
                             .arg(fSetName.c_str()).arg(fElementName.c_str()));

    std::vector<ST::string> existingLanguages;
    existingLanguages = pfLocalizationDataMgr::Instance().GetLanguages(fAgeName, fSetName, fElementName);

    std::vector<ST::string> missingLanguages;
    for (const auto &langName : plLocalization::GetAllLanguageNames()) {
        if (std::find(existingLanguages.begin(), existingLanguages.end(), langName) == existingLanguages.end()) {
            missingLanguages.push_back(langName);
        }
    }

    // see if any languages are missing
    if (missingLanguages.size() == 0)
    {
        // none are missing, so close the dialog
        return false;
    }

    // add the missing languages to the list
    for (int i = 0; i < missingLanguages.size(); i++)
        fUI->fLanguage->addItem(missingLanguages[i].c_str());

    return exec() == QDialog::Accepted;
}

void plAddLocalizationDlg::SelectLanguage(int which)
{
    fLanguageName = fUI->fLanguage->itemText(which).toUtf8().constData();
}
