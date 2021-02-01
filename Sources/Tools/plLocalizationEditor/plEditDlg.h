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
// Basic edit dialog stuff
#ifndef _pfEditDlg_h
#define _pfEditDlg_h

#include <QMainWindow>
#include <string_theory/string>

class QTreeWidgetItem;

class EditDialog : public QMainWindow
{
    Q_OBJECT

public:
    EditDialog();
    virtual ~EditDialog();

    void SetTitle(const QString &path)
    {
        QString title = "plLocalizationEditor";
        if (!path.isEmpty())
            title += " - " + path;
        setWindowTitle(title);
    }

    void SaveLocalizationText();
    void LoadLocalization(const ST::string &path);
    void EnableEdit(bool enable);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void OpenDataDirectory();
    void SaveToCurrent();
    void SaveToDirectory();

    void LocPathChanged(QTreeWidgetItem *current, QTreeWidgetItem *);
    void AddClicked();
    void DeleteClicked();

private:
    class Ui_EditDialog *fUI;
    QString fCurrentSavePath;
    ST::string fCurrentLocPath;

    enum { kEditNothing, kEditElement, kEditLocalization } fEditMode;
};

// Little trick to show a wait cursor while something is working
class plWaitCursor
{
    QWidget *fParent;

public:
    plWaitCursor(QWidget *parent) : fParent(parent)
    {
        fParent->setCursor(Qt::WaitCursor);
    }

    ~plWaitCursor()
    {
        fParent->unsetCursor();
    }
};

void SplitLocalizationPath(const ST::string &path, ST::string &ageName,
        ST::string &setName, ST::string &locName, ST::string &locLanguage);

#endif
