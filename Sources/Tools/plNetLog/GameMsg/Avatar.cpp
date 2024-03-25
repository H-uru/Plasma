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

#include "Avatar.h"
#include "Factory.h"

static void AnimStage_Aux(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    new QTreeWidgetItem(parent, QStringList()
        << QString("Local Time: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Length: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Current Loop: %1").arg(buffer.read<int>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Attached: %1").arg(buffer.read<bool>() ? "True" : "False"));
}

void Create_AnimStage(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_notifyTypeNames[] = {
        "kNotifyEnter", "kNotifyLoop", "kNotifyAdvance", "kNotifyRegress",
        "(1<<4)", "(1<<5)", "(1<<6)", "(1<<7)",
    };
    static const char* s_fwdBackTypes[] = {
        "kNone", "kKey", "kAuto",
    };
    static const char* s_advanceTypes[] = {
        "kNone", "kOnMove", "kAuto", "kOnAnyKey",
    };

    new QTreeWidgetItem(parent, QStringList()
        << QString("Anim Name: %1").arg(buffer.readSafeString()));
    FlagField(parent, "Notify Type", buffer.read<unsigned char>(), s_notifyTypeNames);

    unsigned forwardType = buffer.read<unsigned>();
    if (forwardType < (sizeof(s_fwdBackTypes) / sizeof(s_fwdBackTypes[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Forward Type: %1").arg(s_fwdBackTypes[forwardType]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Forward Type: %1").arg(forwardType));
        item->setForeground(0, Qt::red);
    }

    unsigned backType = buffer.read<unsigned>();
    if (backType < (sizeof(s_fwdBackTypes) / sizeof(s_fwdBackTypes[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Back Type: %1").arg(s_fwdBackTypes[backType]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Back Type: %1").arg(backType));
        item->setForeground(0, Qt::red);
    }

    unsigned advanceType = buffer.read<unsigned>();
    if (advanceType < (sizeof(s_advanceTypes) / sizeof(s_advanceTypes[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Advance Type: %1").arg(s_advanceTypes[advanceType]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Advance Type: %1").arg(advanceType));
        item->setForeground(0, Qt::red);
    }

    unsigned regressType = buffer.read<unsigned>();
    if (regressType < (sizeof(s_advanceTypes) / sizeof(s_advanceTypes[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Regress Type: %1").arg(s_advanceTypes[regressType]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Regress Type: %1").arg(regressType));
        item->setForeground(0, Qt::red);
    }

    new QTreeWidgetItem(parent, QStringList()
        << QString("Loops: %1").arg(buffer.read<int>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Do Advance: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Advance To: %1").arg(buffer.read<int>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Do Regress: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Regress To: %1").arg(buffer.read<int>()));
}


void Create_ArmatureBrain(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    new QTreeWidgetItem(parent, QStringList()
        << QString("Ignored: %1").arg(buffer.read<unsigned>()));
    if (buffer.read<bool>())
        Key(parent, "Ignored Key", buffer);
    new QTreeWidgetItem(parent, QStringList()
        << QString("Ignored: %1").arg(buffer.read<unsigned>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Ignored: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Ignored: %1").arg(buffer.read<double>()));
}

void Create_AvBrainClimb(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_ArmatureBrain(new QTreeWidgetItem(parent, QStringList() << "<plArmatureBrain>"), buffer);
}

void Create_AvBrainCritter(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_ArmatureBrain(new QTreeWidgetItem(parent, QStringList() << "<plArmatureBrain>"), buffer);
}

void Create_AvBrainDrive(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_ArmatureBrain(new QTreeWidgetItem(parent, QStringList() << "<plArmatureBrain>"), buffer);
}

void Create_AvBrainGeneric(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    static const char* s_typeNames[] = {
        "kGeneric", "kLadder", "kSit", "kSitOnGround", "kEmote", "kAFK",
        "kNonGeneric",
    };
    static const char* s_exitFlagNames[] = {
        "kExitAnyTask", "kExitNewBrain", "kExitAnyInput", "(1<<3)",
        "(1<<4)", "(1<<5)", "(1<<6)", "(1<<7)",
        "(1<<8)", "(1<<9)", "(1<<10)", "(1<<11)",
        "(1<<12)", "(1<<13)", "(1<<14)", "(1<<15)",
        "(1<<16)", "(1<<17)", "(1<<18)", "(1<<19)",
        "(1<<20)", "(1<<21)", "(1<<22)", "(1<<23)",
        "(1<<24)", "(1<<25)", "(1<<26)", "(1<<27)",
        "(1<<28)", "(1<<29)", "(1<<30)", "(1<<31)",
    };
    static const char* s_modeNames[] = {
        "kEntering", "kNormal", "kFadingIn", "kFadingOut", "kExit", "kAbort",
    };
    static const char* s_moveModes[] = {
        "kMoveAbsolute", "kMoveRelative", "kMoveNormal", "kMoveStandstill",
    };
    static const char* s_bodyUsages[] = {
        "kBodyUnknown", "kBodyUpper", "kBodyFull", "kBodyLower",
    };

    Create_ArmatureBrain(new QTreeWidgetItem(parent, QStringList() << "<plArmatureBrain>"), buffer);

    QTreeWidgetItem* stages = new QTreeWidgetItem(parent, QStringList() << "Stages");
    unsigned count = buffer.read<unsigned>();
    for (unsigned i = 0; i < count; ++i) {
        QTreeWidgetItem* stage = new QTreeWidgetItem(stages, QStringList());
        QString stageType = Factory_Create(stage, buffer, 0);
        stage->setText(0, QString("Stage %1: %2").arg(i).arg(stageType));
        AnimStage_Aux(stage, buffer);
    }
    new QTreeWidgetItem(parent, QStringList()
        << QString("Current Stage: %1").arg(buffer.read<int>()));

    unsigned type = buffer.read<unsigned>();
    if (type < (sizeof(s_typeNames) / sizeof(s_typeNames[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Type: %1").arg(s_typeNames[type]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Type: %1").arg(type));
        item->setForeground(0, Qt::red);
    }

    FlagField(parent, "Exit Flags", buffer.read<unsigned>(), s_exitFlagNames);

    unsigned char mode = buffer.read<unsigned char>();
    if (mode > 0 && (mode - 1) < (sizeof(s_modeNames) / sizeof(s_modeNames[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Mode: %1").arg(s_modeNames[mode - 1]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Mode: %1").arg(mode));
        item->setForeground(0, Qt::red);
    }

    new QTreeWidgetItem(parent, QStringList()
        << QString("Forward: %1").arg(buffer.read<bool>() ? "True" : "False"));

    if (buffer.read<bool>()) {
        QTreeWidgetItem* start = new QTreeWidgetItem(parent, QStringList());
        QString startType = Factory_Create(start, buffer, 0);
        start->setText(0, QString("Start Message: %1").arg(startType));
    }

    if (buffer.read<bool>()) {
        QTreeWidgetItem* end = new QTreeWidgetItem(parent, QStringList());
        QString endType = Factory_Create(end, buffer, 0);
        end->setText(0, QString("End Message: %1").arg(endType));
    }

    new QTreeWidgetItem(parent, QStringList()
        << QString("Fade In: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Fade Out: %1").arg(buffer.read<float>()));

    unsigned char moveMode = buffer.read<unsigned char>();
    if (moveMode < (sizeof(s_moveModes) / sizeof(s_moveModes[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Move Mode: %1").arg(s_moveModes[moveMode]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Move Mode: %1").arg(moveMode));
        item->setForeground(0, Qt::red);
    }

    unsigned char bodyUsage = buffer.read<unsigned char>();
    if (bodyUsage < (sizeof(s_bodyUsages) / sizeof(s_bodyUsages[0]))) {
        new QTreeWidgetItem(parent, QStringList()
            << QString("Body Usage: %1").arg(s_bodyUsages[bodyUsage]));
    } else {
        QTreeWidgetItem* item = new QTreeWidgetItem(parent, QStringList()
            << QString("Body Usage: %1").arg(bodyUsage));
        item->setForeground(0, Qt::red);
    }

    Key(parent, "Recipient", buffer);
}

void Create_AvBrainHuman(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_ArmatureBrain(new QTreeWidgetItem(parent, QStringList() << "<plArmatureBrain>"), buffer);
    new QTreeWidgetItem(parent, QStringList()
        << QString("Is Custom Avatar: %1").arg(buffer.read<bool>() ? "True" : "False"));
}

void Create_AvBrainRideAnimatedPhysical(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_ArmatureBrain(new QTreeWidgetItem(parent, QStringList() << "<plArmatureBrain>"), buffer);
}

void Create_AvBrainSwim(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    Create_ArmatureBrain(new QTreeWidgetItem(parent, QStringList() << "<plArmatureBrain>"), buffer);
}


void Create_AvAnimTask(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    new QTreeWidgetItem(parent, QStringList()
        << QString("Animation Name: %1").arg(buffer.readSafeString()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Initial Blend: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Target Blend: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Fade Speed: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Set Time: %1").arg(buffer.read<float>()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Start: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Loop: %1").arg(buffer.read<bool>() ? "True" : "False"));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Attach: %1").arg(buffer.read<bool>() ? "True" : "False"));
}

void Create_AvOneShotTask(QTreeWidgetItem* parent, ChunkBuffer& buffer) { }

void Create_AvOneShotLinkTask(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    new QTreeWidgetItem(parent, QStringList()
        << QString("Animation Name: %1").arg(buffer.readSafeString()));
    new QTreeWidgetItem(parent, QStringList()
        << QString("Marker Name: %1").arg(buffer.readSafeString()));
}

void Create_AvSeekTask(QTreeWidgetItem* parent, ChunkBuffer& buffer) { }

void Create_AvTaskBrain(QTreeWidgetItem* parent, ChunkBuffer& buffer)
{
    QTreeWidgetItem* brain = new QTreeWidgetItem(parent, QStringList());
    QString brainType = Factory_Create(brain, buffer, 0);
    brain->setText(0, QString("Brain: %1").arg(brainType));
}

void Create_AvTaskSeek(QTreeWidgetItem* parent, ChunkBuffer& buffer) { }
