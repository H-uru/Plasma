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
#include "pyNetClientComm.h"
#include "../pfPython/pyAgeLinkStruct.h"
#include "../pfPython/pyNetServerSessionInfo.h"
#include "../pfPython/pyStatusLog.h"
#include "../plNetCommon/plCreatePlayerFlags.h"
#include "../pnNetCommon/plGenericVar.h"
#include "hsStlUtils.h"
#include "hsTimer.h"

#include <python.h>

////////////////////////////////////////////////////////////////////

class pyNetClientCommCallback : public plNetClientComm::Callback
{
public:
    PyObject * fPyObject;
    pyNetClientCommCallback( PyObject * pyObject )
        : fPyObject( pyObject )
    {
        Py_XINCREF( fPyObject );
    }
    ~pyNetClientCommCallback()
    {
        Py_XDECREF( fPyObject );
    }
    void OperationStarted( uint32_t context )
    {
        if ( fPyObject )
        {
            // Call the callback.
            PyObject* func = PyObject_GetAttrString( fPyObject, "operationStarted" );
            if ( func )
            {
                if ( PyCallable_Check(func)>0 )
                {
                    PyObject* retVal = PyObject_CallMethod(fPyObject, "operationStarted", "l", context);
                    Py_XDECREF(retVal);
                }
            }
        }
    }
    void OperationComplete( uint32_t context, int resultCode )
    {
        if ( fPyObject )
        {
            // Pass args.
            PyObject* pyArgs = PyObject_GetAttrString( fPyObject, "fCbArgs" );
            if ( pyArgs )
            {
                PyObject* pyDict = PyDict_New();
                std::map<uint16_t,plCreatable*>   args;
                fCbArgs.GetItems( args );
                for ( std::map<uint16_t,plCreatable*>::iterator ii=args.begin(); ii!=args.end(); ++ii )
                {
                    uint16_t key = ii->first;
                    PyObject* keyObj = PyInt_FromLong(key);
                    char* strTemp = NULL;
                    plCreatable* arg = ii->second;
                    plCreatableGenericValue * genValue = plCreatableGenericValue::ConvertNoRef( arg );
                    if ( genValue )
                    {
                        PyObject* valueObj;
                        plGenericType & value = genValue->Value();
                        switch ( value.GetType() )
                        {
                        case plGenericType::kInt:
                            valueObj = PyLong_FromLong((int32_t)value);
                            PyDict_SetItem(pyDict, keyObj, valueObj);
                            Py_DECREF(valueObj);
                            break;
                        case plGenericType::kUInt:
                            valueObj = PyLong_FromUnsignedLong((uint32_t)value);
                            PyDict_SetItem(pyDict, keyObj, valueObj);
                            Py_DECREF(valueObj);
                            break;
                        case plGenericType::kFloat:
                            valueObj = PyFloat_FromDouble((float)value);
                            PyDict_SetItem(pyDict, keyObj, valueObj);
                            Py_DECREF(valueObj);
                            break;
                        case plGenericType::kDouble:
                            valueObj = PyFloat_FromDouble((double)value);
                            PyDict_SetItem(pyDict, keyObj, valueObj);
                            Py_DECREF(valueObj);
                            break;
                        case plGenericType::kBool:
                            if ((bool)value)
                                valueObj = PyInt_FromLong(1);
                            else
                                valueObj = PyInt_FromLong(0);
                            PyDict_SetItem(pyDict, keyObj, valueObj);
                            Py_DECREF(valueObj);
                            break;
                        case plGenericType::kChar:
                            strTemp = new char[2];
                            strTemp[0] = (char)value;
                            strTemp[1] = 0;
                            valueObj = PyString_FromString(strTemp);
                            PyDict_SetItem(pyDict, keyObj, valueObj);
                            Py_DECREF(valueObj);
                            delete [] strTemp;
                            break;
                        case plGenericType::kString:
                            valueObj = PyString_FromString((const char*)value);
                            PyDict_SetItem(pyDict, keyObj, valueObj);
                            Py_DECREF(valueObj);
                            break;
                        case plGenericType::kAny:
                            break;
                        case plGenericType::kNone:
                            break;
                        }
                    }
                    plNetServerSessionInfo * serverInfo = plNetServerSessionInfo::ConvertNoRef( arg );
                    if ( serverInfo )
                    {
                        PyObject* valueObj = pyNetServerSessionInfo::New(*serverInfo);
                        PyDict_SetItem(pyDict, keyObj, valueObj);
                        Py_DECREF(valueObj);
                    }
                    Py_DECREF(keyObj);
                }
                PyObject_SetAttrString( fPyObject, "fCbArgs", pyDict );
                Py_DECREF(pyDict);
            }

            // Call the callback.
            PyObject* func = PyObject_GetAttrString( fPyObject, "operationComplete" );
            if ( func )
            {
                if ( PyCallable_Check(func)>0 )
                {
                    PyObject* retVal = PyObject_CallMethod(fPyObject, "operationComplete", "li", context, resultCode);
                    Py_XDECREF(retVal);
                }
            }
        }
        delete this;
    }
};

////////////////////////////////////////////////////////////////////

// Error handler - throws exception in python script
class pyNetClientCommErrorHandler : public plNetClientComm::ErrorHandler
{
public:
    void HandleError( Error err, int result )
    {
        std::string msg;
        xtl::format( msg, "pyNetClientComm: Error: %s", plNetClientComm::ErrorHandler::ErrorStr( err ) );
        PyErr_SetString(PyExc_KeyError, msg.c_str());
    }
} ThePyNetClientCommErrorHandler;

////////////////////////////////////////////////////////////////////

// pyNetClientComm ----------------------------------------------
pyNetClientComm::pyNetClientComm()
{
    fNetClient.SetErrorHandler( &ThePyNetClientCommErrorHandler );
}

// ~pyNetClientComm ----------------------------------------------
pyNetClientComm::~pyNetClientComm()
{
}

// NetAuthenticate ----------------------------------------------
int pyNetClientComm::NetAuthenticate( double maxAuthSecs, PyObject* cbClass/*=nil*/, uint32_t cbContext/*=0 */)
{
    return fNetClient.NetAuthenticate( maxAuthSecs, new pyNetClientCommCallback( cbClass ), cbContext );
}

// NetLeave ----------------------------------------------
int pyNetClientComm::NetLeave( uint8_t reason, PyObject* cbClass/*=nil*/, uint32_t cbContext/*=0 */)
{
    return fNetClient.NetLeave( reason, new pyNetClientCommCallback( cbClass ), cbContext );
}

// NetPing ----------------------------------------------
int pyNetClientComm::NetPing( int serverType, int timeoutSecs/*=0*/, PyObject* cbClass/*=nil*/, uint32_t cbContext/*=0 */)
{
    return fNetClient.NetPing( serverType, timeoutSecs, new pyNetClientCommCallback( cbClass ), cbContext );
}

// NetFindAge ----------------------------------------------
int pyNetClientComm::NetFindAge( const pyAgeLinkStruct* linkInfo, PyObject* cbClass/*=nil*/, uint32_t cbContext/*=0 */)
{
    return fNetClient.NetFindAge( linkInfo->GetAgeLink(), new pyNetClientCommCallback( cbClass ), cbContext );
}

// NetGetPlayerList ----------------------------------------------
int pyNetClientComm::NetGetPlayerList( PyObject* cbClass/*=nil*/, uint32_t cbContext/*=0 */)
{
    return fNetClient.NetGetPlayerList( new pyNetClientCommCallback( cbClass ), cbContext );
}

// NetSetActivePlayer ----------------------------------------------
int pyNetClientComm::NetSetActivePlayer( uint32_t playerID, const char* playerName, PyObject* cbClass/*=nil*/, uint32_t cbContext/*=0 */)
{
    return fNetClient.NetSetActivePlayer( playerID, playerName, 0 /*ccrLevel*/, new pyNetClientCommCallback( cbClass ), cbContext );
}

// NetCreatePlayer ----------------------------------------------
int pyNetClientComm::NetCreatePlayer( const char* playerName, const char* avatarShape, uint32_t createFlags, PyObject* cbClass/*=nil*/, uint32_t cbContext/*=0 */)
{
    return fNetClient.NetCreatePlayer( playerName, avatarShape, createFlags, nil, nil, nil, new pyNetClientCommCallback( cbClass ), cbContext );
}

// NetJoinAge ----------------------------------------------
int pyNetClientComm::NetJoinAge( PyObject* cbClass/*=nil*/, uint32_t cbContext/*=0 */)
{
    return fNetClient.NetJoinAge( true /*tryP2P*/, true /*allowTimeout*/, new pyNetClientCommCallback( cbClass ), cbContext );
}

// NetSetTimeout ----------------------------------------------
int pyNetClientComm::NetSetTimeout( float timeoutSecs, PyObject* cbClass/*=nil*/, uint32_t cbContext/*=0 */)
{
    return fNetClient.NetSetTimeout( timeoutSecs, new pyNetClientCommCallback( cbClass ), cbContext );
}

// SetLogLevel ----------------------------------------------
void pyNetClientComm::SetLogLevel( int logLevel )
{
    fNetClient.SetLogLevel( logLevel );
}

// Startup ----------------------------------------------
int pyNetClientComm::Init( bool threaded/*=true */, int logLevel/*=0 */)
{
    return fNetClient.Init( threaded, logLevel );
}

// Shutdown ----------------------------------------------
int pyNetClientComm::Fini( float flushMsgsSecs/*=0.f */)
{
    return fNetClient.Fini( flushMsgsSecs );
}

// Update ----------------------------------------------
int pyNetClientComm::Update()
{
    return fNetClient.Update( hsTimer::GetSeconds() );
}

// SetActiveServer ----------------------------------------------
int pyNetClientComm::SetActiveServer( pyNetServerSessionInfo* nfo )
{
    return fNetClient.SetActiveServer( &nfo->ServerInfo() );
}

// SetActiveServer2 ----------------------------------------------
int pyNetClientComm::SetActiveServer2( const char * addr, int port )
{
    plNetServerSessionInfo nfo;
    nfo.SetServerAddr( addr );
    nfo.SetServerPort( port );
    return fNetClient.SetActiveServer( &nfo );
}


// SetAuthInfo ----------------------------------------------
int pyNetClientComm::SetAuthInfo( const char* acctName, const char* password )
{
    return fNetClient.SetAuthInfo( acctName, password );
}

// SetLogByName ----------------------------------------------
void pyNetClientComm::SetLogByName( const char * name, uint32_t flags )
{
    plStatusLog * log = plStatusLogMgr::GetInstance().CreateStatusLog( 80, name,
        flags | plStatusLog::kTimestamp | plStatusLog::kDeleteForMe );
    fNetClient.SetLog( log );
}

// GetLog ----------------------------------------------
PyObject* pyNetClientComm::GetLog() const
{
    return pyStatusLog::New( fNetClient.GetLog() );
}

// SetServerSilenceTime ----------------------------------------------
void pyNetClientComm::SetServerSilenceTime( float secs )
{
    fNetClient.SetServerSilenceTime( secs );
}

////////////////////////////////////////////////////////////////////
// End.
