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

#include <Python.h>
#include "hsTimer.h"
#pragma hdrstop

#include "pyAlarm.h"

////////////////////////////////////////////////////////////////////

struct pyAlarm
{
    double  fStart;
    float   fSecs;
    PyObject *  fCb;
    uint32_t  fCbContext;
    pyAlarm( double start, float secs, PyObject * cb, uint32_t cbContext )
    : fStart( start )
    , fSecs( secs )
    , fCb( cb )
    , fCbContext( cbContext )
    {
        Py_XINCREF( fCb );
    }
    ~pyAlarm()
    {
        Py_XDECREF( fCb );
    }
    bool MaybeFire( double secs )
    {
        if ( secs-fStart>fSecs )
        {
            Fire();
            return true;
        }
        return false;
    }
    void Fire()
    {
        if ( fCb )
        {
            PyObject* func = nil;

            // Call the callback.
            func = PyObject_GetAttrString( fCb, "onAlarm" );
            if ( func )
            {
                if ( PyCallable_Check(func)>0 )
                {
                    PyObject *retVal = PyObject_CallMethod(fCb, "onAlarm", "l", fCbContext);
                    Py_XDECREF(retVal);
                }
            }
            Py_XDECREF(func);
        }
    }
};


////////////////////////////////////////////////////////////////////

//static
pyAlarmMgr * pyAlarmMgr::GetInstance()
{
    static pyAlarmMgr inst;
    return &inst;
}

pyAlarmMgr::~pyAlarmMgr()
{
//  Clear();
}

void pyAlarmMgr::Update( double secs )
{
    Alarms::iterator it = fAlarms.begin();
    while ( it!=fAlarms.end() )
    {
        pyAlarm * alarm = (*it);
        if ( alarm->MaybeFire( secs ) )
        {
            Alarms::iterator jt = it++;
            fAlarms.erase( jt );
            delete alarm;
        }
        else
        {
            it++;
        }
    }
}

void pyAlarmMgr::SetAlarm( float secs, PyObject * cb, uint32_t cbContext )
{
    double start = hsTimer::GetSysSeconds();
    fAlarms.push_back( new pyAlarm( start, secs, cb, cbContext ) );
}

void pyAlarmMgr::Clear()
{
    for (Alarms::iterator i = fAlarms.begin(); i != fAlarms.end(); i++)
        delete *i;
    fAlarms.clear();
}


