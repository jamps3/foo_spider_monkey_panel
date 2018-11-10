#include <stdafx.h>
#include "drop_source_impl.h"

#include <com_objects/internal/drag_image.h>
#include <com_objects/internal/drag_utils.h>
#include <js_utils/winapi_error_helper.h>

_COM_SMARTPTR_TYPEDEF( IDragSourceHelper2, IID_IDragSourceHelper2 );

namespace smp::com
{

IDropSourceImpl::IDropSourceImpl( HWND hWnd, IDataObject* pDataObject, size_t itemCount )
    : pDataObject_( pDataObject )
{
    assert( hWnd );
    assert( pDataObject );
    assert( itemCount );

    HRESULT hr = pDragSourceHelper_.CreateInstance( CLSID_DragDropHelper, nullptr, CLSCTX_INPROC_SERVER );
    mozjs::error::CheckHR( hr, "CreateInstance" );

    if ( IDragSourceHelper2Ptr pDragSourceHelper2 = pDragSourceHelper_;
         pDragSourceHelper2 )
    {
        (void)pDragSourceHelper2->SetFlags( DSH_ALLOWDROPDESCRIPTIONTEXT );
    }

    if ( drag::RenderDragImage( hWnd, itemCount, dragImage_ ) )
    {
        (void)pDragSourceHelper_->InitializeFromBitmap( &dragImage_, pDataObject );
    }

    if ( IsThemeActive() && IsAppThemed() )
    {
        (void)drag::SetDefaultImage( pDataObject );
    }
};

IDropSourceImpl::~IDropSourceImpl()
{
    if ( dragImage_.hbmpDragImage )
    {
        DeleteObject( dragImage_.hbmpDragImage );
    }
}

STDMETHODIMP IDropSourceImpl::QueryContinueDrag( BOOL fEscapePressed, DWORD grfKeyState )
{
    if ( fEscapePressed || ( grfKeyState & MK_RBUTTON ) || ( grfKeyState & MK_MBUTTON ) )
    {
        return DRAGDROP_S_CANCEL;
    }

    if ( !( grfKeyState & MK_LBUTTON ) )
    {
        return m_dwLastEffect == DROPEFFECT_NONE ? DRAGDROP_S_CANCEL : DRAGDROP_S_DROP;
    }

    return S_OK;
}

STDMETHODIMP IDropSourceImpl::GiveFeedback( DWORD dwEffect )
{
    BOOL isShowingLayered = FALSE;
    if ( IsThemeActive() )
    {
        drag::GetIsShowingLayered( pDataObject_, isShowingLayered );
    }

    HWND wnd_drag = nullptr;
    if ( SUCCEEDED( drag::GetDragWindow( pDataObject_, wnd_drag ) ) && wnd_drag )
    {
        PostMessage( wnd_drag, DDWM_UPDATEWINDOW, NULL, NULL );
    }

    if ( isShowingLayered )
    {
        if ( !wasShowingLayered_ )
        {
            SetCursor( LoadCursor( nullptr, IDC_ARROW ) );
        }
        if ( wnd_drag && dwEffect == DROPEFFECT_NONE )
        { 
            /*
            WPARAM wp = 1;
            if ( dwEffect & DROPEFFECT_MOVE )
            {
                wp = 2;
            }
            else if ( dwEffect & DROPEFFECT_COPY )
            {
                wp = 3;
            }
            else if ( dwEffect & DROPEFFECT_LINK )
            {
                wp = 4;
            }
            */
            PostMessage( wnd_drag, DDWM_SETCURSOR, 1 /*wp*/, NULL );
        }
    }

    wasShowingLayered_ = !!isShowingLayered;
    m_dwLastEffect = dwEffect;

    return ( isShowingLayered ? S_OK : DRAGDROP_S_USEDEFAULTCURSORS );
}

ULONG STDMETHODCALLTYPE IDropSourceImpl::AddRef()
{
    return InterlockedIncrement( &m_refCount );
}

ULONG STDMETHODCALLTYPE IDropSourceImpl::Release()
{
    LONG rv = InterlockedDecrement( &m_refCount );
    if ( !rv )
    {
        delete this;
    }
    return rv;
}

} // namespace smp::com