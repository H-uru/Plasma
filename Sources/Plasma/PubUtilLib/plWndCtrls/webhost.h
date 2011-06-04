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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
// Stolen from: http://www.mvps.org/user32/webhost.cab
// No copyright notices, so I assume it's public domain -Colin
#include <windows.h>
#include "basewnd.h"
#include <shlobj.h>
#include <lmcons.h>
#include <exdisp.h>

#define NOTIMPLEMENTED __asm{ int 3 }; return E_NOTIMPL


struct CNullStorage
: public IStorage
{
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid,void ** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	// IStorage
	STDMETHODIMP CreateStream(const WCHAR * pwcsName,DWORD grfMode,DWORD reserved1,DWORD reserved2,IStream ** ppstm);
	STDMETHODIMP OpenStream(const WCHAR * pwcsName,void * reserved1,DWORD grfMode,DWORD reserved2,IStream ** ppstm);
	STDMETHODIMP CreateStorage(const WCHAR * pwcsName,DWORD grfMode,DWORD reserved1,DWORD reserved2,IStorage ** ppstg);
	STDMETHODIMP OpenStorage(const WCHAR * pwcsName,IStorage * pstgPriority,DWORD grfMode,SNB snbExclude,DWORD reserved,IStorage ** ppstg);
	STDMETHODIMP CopyTo(DWORD ciidExclude,IID const * rgiidExclude,SNB snbExclude,IStorage * pstgDest);
	STDMETHODIMP MoveElementTo(const OLECHAR * pwcsName,IStorage * pstgDest,const OLECHAR* pwcsNewName,DWORD grfFlags);
	STDMETHODIMP Commit(DWORD grfCommitFlags);
	STDMETHODIMP Revert(void);
	STDMETHODIMP EnumElements(DWORD reserved1,void * reserved2,DWORD reserved3,IEnumSTATSTG ** ppenum);
	STDMETHODIMP DestroyElement(const OLECHAR * pwcsName);
	STDMETHODIMP RenameElement(const WCHAR * pwcsOldName,const WCHAR * pwcsNewName);
	STDMETHODIMP SetElementTimes(const WCHAR * pwcsName,FILETIME const * pctime,FILETIME const * patime,FILETIME const * pmtime);
	STDMETHODIMP SetClass(REFCLSID clsid);
	STDMETHODIMP SetStateBits(DWORD grfStateBits,DWORD grfMask);
	STDMETHODIMP Stat(STATSTG * pstatstg,DWORD grfStatFlag);
};

struct webhostwnd;


struct CMyFrame
: public IOleInPlaceFrame
{
	webhostwnd* host;
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid,void ** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	// IOleWindow
	STDMETHODIMP GetWindow(HWND FAR* lphwnd);
	STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
	// IOleInPlaceUIWindow
	STDMETHODIMP GetBorder(LPRECT lprectBorder);
	STDMETHODIMP RequestBorderSpace(LPCBORDERWIDTHS pborderwidths);
	STDMETHODIMP SetBorderSpace(LPCBORDERWIDTHS pborderwidths);
	STDMETHODIMP SetActiveObject(IOleInPlaceActiveObject *pActiveObject,LPCOLESTR pszObjName);
	// IOleInPlaceFrame
	STDMETHODIMP InsertMenus(HMENU hmenuShared,LPOLEMENUGROUPWIDTHS lpMenuWidths);
	STDMETHODIMP SetMenu(HMENU hmenuShared,HOLEMENU holemenu,HWND hwndActiveObject);
	STDMETHODIMP RemoveMenus(HMENU hmenuShared);
	STDMETHODIMP SetStatusText(LPCOLESTR pszStatusText);
	STDMETHODIMP EnableModeless(BOOL fEnable);
	STDMETHODIMP TranslateAccelerator(  LPMSG lpmsg,WORD wID);
	
};



struct CMySite
: public IOleClientSite,
public IOleInPlaceSite
{
	webhostwnd* host;
	
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid,void ** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	// IOleClientSite
	STDMETHODIMP SaveObject();
	STDMETHODIMP GetMoniker(DWORD dwAssign,DWORD dwWhichMoniker,IMoniker ** ppmk);
	STDMETHODIMP GetContainer(LPOLECONTAINER FAR* ppContainer);
	STDMETHODIMP ShowObject();
	STDMETHODIMP OnShowWindow(BOOL fShow);
	STDMETHODIMP RequestNewObjectLayout();
	// IOleWindow
	STDMETHODIMP GetWindow(HWND FAR* lphwnd);
	STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);
	// IOleInPlaceSite methods
	STDMETHODIMP CanInPlaceActivate();
	STDMETHODIMP OnInPlaceActivate();
	STDMETHODIMP OnUIActivate();
	STDMETHODIMP GetWindowContext(LPOLEINPLACEFRAME FAR* lplpFrame,LPOLEINPLACEUIWINDOW FAR* lplpDoc,LPRECT lprcPosRect,LPRECT lprcClipRect,LPOLEINPLACEFRAMEINFO lpFrameInfo);
	STDMETHODIMP Scroll(SIZE scrollExtent);
	STDMETHODIMP OnUIDeactivate(BOOL fUndoable);
	STDMETHODIMP OnInPlaceDeactivate();
	STDMETHODIMP DiscardUndoState();
	STDMETHODIMP DeactivateAndUndo();
	STDMETHODIMP OnPosRectChange(LPCRECT lprcPosRect);
};


struct CMyContainer
: public IOleContainer
{
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid,void ** ppvObject);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);
	// IParseDisplayName
	STDMETHODIMP ParseDisplayName(IBindCtx *pbc,LPOLESTR pszDisplayName,ULONG *pchEaten,IMoniker **ppmkOut);
	// IOleContainer
	STDMETHODIMP EnumObjects(DWORD grfFlags,IEnumUnknown **ppenum);
	STDMETHODIMP LockContainer(BOOL fLock);
};


struct webhostwnd : public basewnd
{
	webhostwnd();
	~webhostwnd();
	
	static webhostwnd* Create(HWND hParent, RECT& rect);
	
	virtual BOOL HandleMessage(UINT,WPARAM,LPARAM,LRESULT*);
	
	void CreateEmbeddedWebControl(void);
	void UnCreateEmbeddedWebControl(void);
	
	HWND operator =(webhostwnd* rhs);
	
	CNullStorage storage;
	CMySite site;
	CMyFrame frame;
	
	IOleObject* mpWebObject;
	
	void OnPaint(HDC hdc);
	void GoToPage(const char* address);
};




