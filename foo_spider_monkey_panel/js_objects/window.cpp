#include <stdafx.h>
#include "window.h"

#include <js_engine/js_to_native_invoker.h>
#include <js_objects/menu_object.h>
#include <js_objects/theme_manager.h>
#include <js_objects/fb_tooltip.h>
#include <js_objects/gdi_font.h>
#include <js_objects/internal/fb_properties.h>
#include <js_utils/js_error_helper.h>
#include <js_utils/winapi_error_helper.h>
#include <js_utils/js_object_helper.h>
#include <js_utils/scope_helper.h>

#include <js_panel_window.h>
#include <panel_manager.h>
#include <helpers.h>
#include <user_message.h>
#include <host_drop_target.h>

namespace
{

using namespace mozjs;

JSClassOps jsOps = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    JsWindow::FinalizeJsObject,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

JSClass jsClass = {
    "Window",
    DefaultClassFlags(),
    &jsOps
};

MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, ClearInterval )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, ClearTimeout )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, CreatePopupMenu )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, CreateThemeManager )
MJS_DEFINE_JS_TO_NATIVE_FN_WITH_OPT( JsWindow, CreateTooltip, CreateTooltipWithOpt, 3 )
MJS_DEFINE_JS_TO_NATIVE_FN_WITH_OPT( JsWindow, DefinePanel, DefinePanelWithOpt, 1 )
MJS_DEFINE_JS_TO_NATIVE_FN_WITH_OPT( JsWindow, GetColourCUI, GetColourCUIWithOpt, 1 )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, GetColourDUI )
MJS_DEFINE_JS_TO_NATIVE_FN_WITH_OPT( JsWindow, GetFontCUI, GetFontCUIWithOpt, 1 )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, GetFontDUI )
MJS_DEFINE_JS_TO_NATIVE_FN_WITH_OPT( JsWindow, GetProperty, GetPropertyWithOpt, 1 )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, NotifyOthers )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, Reload )
MJS_DEFINE_JS_TO_NATIVE_FN_WITH_OPT( JsWindow, Repaint, RepaintWithOpt, 1 )
MJS_DEFINE_JS_TO_NATIVE_FN_WITH_OPT( JsWindow, RepaintRect, RepaintRectWithOpt, 1 )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, SetCursor )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, SetInterval )
MJS_DEFINE_JS_TO_NATIVE_FN_WITH_OPT( JsWindow, SetProperty, SetPropertyWithOpt, 1 )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, SetTimeout )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, ShowConfigure )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, ShowProperties )

const JSFunctionSpec jsFunctions[] = {
    JS_FN( "ClearInterval", ClearInterval, 1, DefaultPropsFlags() ),
    JS_FN( "ClearTimeout", ClearTimeout, 1, DefaultPropsFlags() ),
    JS_FN( "CreatePopupMenu", CreatePopupMenu, 0, DefaultPropsFlags() ),
    JS_FN( "CreateThemeManager", CreateThemeManager, 1, DefaultPropsFlags() ),
    JS_FN( "CreateTooltip", CreateTooltip, 0, DefaultPropsFlags() ),
    JS_FN( "DefinePanel", DefinePanel, 1, DefaultPropsFlags() ),
    JS_FN( "GetColourCUI", GetColourCUI, 1, DefaultPropsFlags() ),
    JS_FN( "GetColourDUI", GetColourDUI, 1, DefaultPropsFlags() ),
    JS_FN( "GetFontCUI", GetFontCUI, 1, DefaultPropsFlags() ),
    JS_FN( "GetFontDUI", GetFontDUI, 1, DefaultPropsFlags() ),
    JS_FN( "GetProperty", GetProperty, 1, DefaultPropsFlags() ),
    JS_FN( "NotifyOthers", NotifyOthers, 2, DefaultPropsFlags() ),
    JS_FN( "Reload", Reload, 0, DefaultPropsFlags() ),
    JS_FN( "Repaint", Repaint, 0, DefaultPropsFlags() ),
    JS_FN( "RepaintRect", RepaintRect, 4, DefaultPropsFlags() ),
    JS_FN( "SetCursor", SetCursor, 1, DefaultPropsFlags() ),
    JS_FN( "SetInterval", SetInterval, 2, DefaultPropsFlags() ),
    JS_FN( "SetProperty", SetProperty, 1, DefaultPropsFlags() ),
    JS_FN( "SetTimeout", SetTimeout, 2, DefaultPropsFlags() ),
    JS_FN( "ShowConfigure", ShowConfigure, 0, DefaultPropsFlags() ),
    JS_FN( "ShowProperties", ShowProperties, 0, DefaultPropsFlags() ),
    JS_FS_END
};

MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_DlgCode )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_Height )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_ID )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_InstanceType )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_IsTransparent )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_IsVisible )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_MaxHeight )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_MaxWidth )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_MinHeight )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_MinWidth )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_Name )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, get_Width )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, put_DlgCode )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, put_MaxHeight )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, put_MaxWidth )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, put_MinHeight )
MJS_DEFINE_JS_TO_NATIVE_FN( JsWindow, put_MinWidth )

const JSPropertySpec jsProperties[] = {
    JS_PSGS( "DlgCode", get_DlgCode, put_DlgCode, DefaultPropsFlags() ),
    JS_PSG( "Height", get_Height, DefaultPropsFlags() ),
    JS_PSG( "ID", get_ID, DefaultPropsFlags() ),
    JS_PSG( "InstanceType", get_InstanceType, DefaultPropsFlags() ),
    JS_PSG( "IsTransparent", get_IsTransparent, DefaultPropsFlags() ),
    JS_PSG( "IsVisible", get_IsVisible, DefaultPropsFlags() ),
    JS_PSGS( "MaxHeight", get_MaxHeight, put_MaxHeight, DefaultPropsFlags() ),
    JS_PSGS( "MaxWidth", get_MaxWidth, put_MaxWidth, DefaultPropsFlags() ),
    JS_PSGS( "MinHeight", get_MinHeight, put_MinHeight, DefaultPropsFlags() ),
    JS_PSGS( "MinWidth", get_MinWidth, put_MinWidth, DefaultPropsFlags() ),
    JS_PSG( "Name", get_Name, DefaultPropsFlags() ),
    JS_PSG( "Width", get_Width, DefaultPropsFlags() ),
    JS_PS_END
};

}

namespace mozjs
{

const JSClass JsWindow::JsClass = jsClass;
const JSFunctionSpec* JsWindow::JsFunctions = jsFunctions;
const JSPropertySpec* JsWindow::JsProperties = jsProperties;

JsWindow::JsWindow( JSContext* cx, js_panel_window& parentPanel, std::unique_ptr<FbProperties> fbProperties )
    : pJsCtx_( cx )
    , parentPanel_( parentPanel )
    , fbProperties_( std::move( fbProperties ) )
{
}

JsWindow::~JsWindow()
{
    CleanupBeforeDestruction();
}

std::unique_ptr<JsWindow>
JsWindow::CreateNative( JSContext* cx, js_panel_window& parentPanel )
{
    std::unique_ptr<FbProperties> fbProperties = FbProperties::Create( cx, parentPanel );
    if ( !fbProperties )
    {// report in Create
        return nullptr;
    }

    return std::unique_ptr<JsWindow>( new JsWindow( cx, parentPanel, std::move( fbProperties ) ));
}

size_t JsWindow::GetInternalSize( const js_panel_window& parentPanel )
{
    return sizeof( FbProperties );
}

void JsWindow::CleanupBeforeDestruction()
{
    if ( fbProperties_ )
    {
        fbProperties_->RemoveHeapTracer();
    }
    if ( dropTargetHandler_ )
    {
        dropTargetHandler_->RevokeDragDrop();
        dropTargetHandler_.Detach();
    }
}

std::optional<std::nullptr_t>
JsWindow::ClearInterval( uint32_t intervalId )
{
    parentPanel_.ClearIntervalOrTimeout( intervalId );
    return nullptr;
}

std::optional<std::nullptr_t>
JsWindow::ClearTimeout( uint32_t timeoutId )
{
    parentPanel_.ClearIntervalOrTimeout( timeoutId );
    return nullptr;
}

std::optional<JSObject*>
JsWindow::CreatePopupMenu()
{
    JS::RootedObject jsObject( pJsCtx_, JsMenuObject::CreateJs( pJsCtx_, parentPanel_.GetHWND() ) );
    if ( !jsObject )
    {// Report in Create
        return std::nullopt;
    }

    return jsObject;
}

std::optional<JSObject*>
JsWindow::CreateThemeManager( const std::wstring& classid )
{
    if ( !JsThemeManager::HasThemeData( parentPanel_.GetHWND(), classid ) )
    {// Not a error: not found
        return nullptr;
    }

    JS::RootedObject jsObject( pJsCtx_, JsThemeManager::CreateJs( pJsCtx_, parentPanel_.GetHWND(), classid ) );
    if ( !jsObject )
    {// Report in Create
        return std::nullopt;
    }

    return jsObject;
}

std::optional<JSObject*>
JsWindow::CreateTooltip( const std::wstring& name, float pxSize, uint32_t style )
{
    auto& tooltip_param = parentPanel_.GetPanelTooltipParam();
    tooltip_param.fontName = name;
    tooltip_param.fontSize = pxSize;
    tooltip_param.fontStyle = style;

    JS::RootedObject jsObject( pJsCtx_, JsFbTooltip::CreateJs( pJsCtx_, parentPanel_.GetHWND(), tooltip_param ) );
    if ( !jsObject )
    {// Report in Create
        return std::nullopt;
    }

    return jsObject;
}

std::optional<JSObject*>
JsWindow::CreateTooltipWithOpt( size_t optArgCount, const std::wstring& name, float pxSize, uint32_t style )
{
    if ( optArgCount > 3 )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Internal error: invalid number of optional arguments specified: %d", optArgCount );
        return std::nullopt;
    }

    if ( optArgCount == 3 )
    {
        return CreateTooltip();
    }
    else if ( optArgCount == 2 )
    {
        return CreateTooltip( name );
    }
    else if ( optArgCount == 1 )
    {
        return CreateTooltip( name, pxSize );
    }

    return CreateTooltip( name, pxSize, style );
}

std::optional<std::nullptr_t> 
JsWindow::DefinePanel( const pfc::string8_fast& name, JS::HandleValue options )
{// TODO: clean up this mess
    if ( isPanelDefined_ )
    {
        JS_ReportErrorUTF8( pJsCtx_, "DefinePanel can't be called twice" );
        return std::nullopt;
    }

    struct Options
    {
        pfc::string8_fast author;
        pfc::string8_fast version;
        struct Features
        {
            bool drag_n_drop = false;
        } features;
    };

    Options parsed_options;
    {
        if ( !options.isNullOrUndefined() )
        {
            if ( !options.isObject() )
            {
                JS_ReportErrorUTF8( pJsCtx_, "options argument is not an object" );
                return std::nullopt;
            }

            JS::RootedObject jsOptions( pJsCtx_, &options.toObject() );

            bool hasProp;
            if ( !JS_HasProperty( pJsCtx_, jsOptions, "author", &hasProp ) )
            {// reports
                return std::nullopt;
            }

            if ( hasProp )
            {
                JS::RootedValue jsValue( pJsCtx_ );
                if ( !JS_GetProperty( pJsCtx_, jsOptions, "author", &jsValue ) )
                {// reports
                    return std::nullopt;
                }

                auto retVal = convert::to_native::ToValue<pfc::string8_fast>( pJsCtx_, jsValue );
                if ( !retVal )
                {
                    JS_ReportErrorUTF8( pJsCtx_, "Failed to parse `author`" );
                    return std::nullopt;
                }

                parsed_options.author = retVal.value();
            }

            if ( !JS_HasProperty( pJsCtx_, jsOptions, "version", &hasProp ) )
            {// reports
                return std::nullopt;
            }

            if ( hasProp )
            {
                JS::RootedValue jsValue( pJsCtx_ );
                if ( !JS_GetProperty( pJsCtx_, jsOptions, "version", &jsValue ) )
                {// reports
                    return std::nullopt;
                }

                auto retVal = convert::to_native::ToValue<pfc::string8_fast>( pJsCtx_, jsValue );
                if ( !retVal )
                {
                    JS_ReportErrorUTF8( pJsCtx_, "Failed to parse `version`" );
                    return std::nullopt;
                }

                parsed_options.version = retVal.value();
            }

            if ( !JS_HasProperty( pJsCtx_, jsOptions, "features", &hasProp ) )
            {// reports
                return std::nullopt;
            }

            if ( hasProp )
            {
                JS::RootedValue jsFeaturesValue( pJsCtx_ );
                if ( !JS_GetProperty( pJsCtx_, jsOptions, "features", &jsFeaturesValue ) )
                {// reports
                    return std::nullopt;
                }

                if ( !jsFeaturesValue.isObject() )
                {
                    JS_ReportErrorUTF8( pJsCtx_, "features is not an object" );
                    return std::nullopt;
                }

                JS::RootedObject jsFeatures( pJsCtx_, &jsFeaturesValue.toObject() );
                if ( !JS_HasProperty( pJsCtx_, jsFeatures, "drag_n_drop", &hasProp ) )
                {// reports
                    return std::nullopt;
                }

                if ( hasProp )
                {
                    JS::RootedValue jsValue( pJsCtx_ );
                    if ( !JS_GetProperty( pJsCtx_, jsFeatures, "drag_n_drop", &jsValue ) )
                    {// reports
                        return std::nullopt;
                    }

                    auto retVal = convert::to_native::ToValue<bool>( pJsCtx_, jsValue );
                    if ( !retVal )
                    {
                        JS_ReportErrorUTF8( pJsCtx_, "`drag_n_drop` can't be converted to boolean" );
                        return std::nullopt;
                    }

                    parsed_options.features.drag_n_drop = retVal.value();
                }
            }            
        }
    }

    parentPanel_.ScriptInfo().name = name;
    parentPanel_.ScriptInfo().author = parsed_options.author;
    parentPanel_.ScriptInfo().version = parsed_options.version;
    if ( parsed_options.features.drag_n_drop )
    {
        dropTargetHandler_.Attach( new com_object_impl_t<HostDropTarget>( parentPanel_.GetHWND() ) );

        HRESULT hr = dropTargetHandler_->RegisterDragDrop();
        IF_HR_FAILED_RETURN_WITH_REPORT( pJsCtx_, hr, std::nullopt, RegisterDragDrop );
    }

    isPanelDefined_ = true;
    return nullptr;
}

std::optional<std::nullptr_t> 
JsWindow::DefinePanelWithOpt( size_t optArgCount, const pfc::string8_fast& name, JS::HandleValue options )
{
    if ( optArgCount > 1 )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Internal error: invalid number of optional arguments specified: %d", optArgCount );
        return std::nullopt;
    }

    if ( optArgCount == 1 )
    {
        return DefinePanel( name );
    }

    return DefinePanel( name, options );
}

std::optional<uint32_t>
JsWindow::GetColourCUI( uint32_t type, const std::wstring& guidstr )
{
    if ( parentPanel_.GetPanelType() != js_panel_window::PanelType::CUI )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Can be called only in CUI" );
        return std::nullopt;
    }

    GUID guid;
    if ( guidstr.empty() )
    {
        memcpy( &guid, &pfc::guid_null, sizeof( guid ) );
    }
    else
    {
        HRESULT hr = CLSIDFromString( guidstr.c_str(), &guid );
        IF_HR_FAILED_RETURN_WITH_REPORT( pJsCtx_, hr, std::nullopt, CLSIDFromString );
    }

    return parentPanel_.GetColourCUI( type, guid );
}

std::optional<uint32_t>
JsWindow::GetColourCUIWithOpt( size_t optArgCount, uint32_t type, const std::wstring& guidstr )
{
    if ( optArgCount > 1 )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Internal error: invalid number of optional arguments specified: %d", optArgCount );
        return std::nullopt;
    }

    if ( optArgCount == 1 )
    {
        return GetColourCUI( type );
    }

    return GetColourCUI( type, guidstr );
}

std::optional<uint32_t>
JsWindow::GetColourDUI( uint32_t type )
{
    if ( parentPanel_.GetPanelType() != js_panel_window::PanelType::DUI )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Can be called only in DUI" );
        return std::nullopt;
    }

    return parentPanel_.GetColourDUI( type );
}

std::optional<JSObject*>
JsWindow::GetFontCUI( uint32_t type, const std::wstring& guidstr )
{
    if ( parentPanel_.GetPanelType() != js_panel_window::PanelType::CUI )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Can be called only in CUI" );
        return std::nullopt;
    }

    GUID guid;
    if ( guidstr.empty() )
    {
        memcpy( &guid, &pfc::guid_null, sizeof( guid ) );
    }
    else
    {
        HRESULT hr = CLSIDFromString( guidstr.c_str(), &guid );
        IF_HR_FAILED_RETURN_WITH_REPORT( pJsCtx_, hr, std::nullopt, CLSIDFromString );
    }

    HFONT hFont = parentPanel_.GetFontCUI( type, guid );
    scope::unique_ptr<std::remove_pointer_t<HFONT>> autoFont( hFont, []( auto obj )
                                                              {
                                                                  DeleteObject( obj );
                                                              } );
    if ( !hFont )
    {// Not an error: font not found
        return nullptr;
    }

    std::unique_ptr<Gdiplus::Font> pGdiFont( new Gdiplus::Font( parentPanel_.GetHDC(), hFont ) );
    if ( !helpers::ensure_gdiplus_object( pGdiFont.get() ) )
    {// Not an error: font not found
        return nullptr;
    }

    JS::RootedObject jsObject( pJsCtx_, JsGdiFont::CreateJs( pJsCtx_, std::move(pGdiFont), hFont, true ) );
    if ( !jsObject )
    {// Report in Create
        return std::nullopt;
    }

    autoFont.release();
    return jsObject;
}

std::optional<JSObject*>
JsWindow::GetFontCUIWithOpt( size_t optArgCount, uint32_t type, const std::wstring& guidstr )
{
    if ( optArgCount > 1 )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Internal error: invalid number of optional arguments specified: %d", optArgCount );
        return std::nullopt;
    }

    if ( optArgCount == 1 )
    {
        return GetFontCUI( type );
    }

    return GetFontCUI( type, guidstr );
}

std::optional<JSObject*>
JsWindow::GetFontDUI( uint32_t type )
{
    if ( parentPanel_.GetPanelType() != js_panel_window::PanelType::DUI )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Can be called only in DUI" );
        return std::nullopt;
    }

    HFONT hFont = parentPanel_.GetFontDUI( type ); // No need to delete, it is managed by DUI
    if ( !hFont )
    {// Not an error: font not found
        return nullptr;
    }

    std::unique_ptr<Gdiplus::Font> pGdiFont( new Gdiplus::Font( parentPanel_.GetHDC(), hFont ) );
    if ( !helpers::ensure_gdiplus_object( pGdiFont.get() ) )
    {// Not an error: font not found
        return nullptr;
    }

    JS::RootedObject jsObject( pJsCtx_, JsGdiFont::CreateJs( pJsCtx_, std::move( pGdiFont ), hFont, false ) );
    if ( !jsObject )
    {// Report in Create
        return std::nullopt;
    }

    return jsObject;
}

std::optional<JS::Heap<JS::Value>>
JsWindow::GetProperty( const std::wstring& name, JS::HandleValue defaultval )
{
    return fbProperties_->GetProperty( name, defaultval );
}

std::optional<JS::Heap<JS::Value>> 
JsWindow::GetPropertyWithOpt( size_t optArgCount, const std::wstring& name, JS::HandleValue defaultval )
{
    if ( optArgCount > 1 )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Internal error: invalid number of optional arguments specified: %d", optArgCount );
        return std::nullopt;
    }

    if ( optArgCount == 1 )
    {
        return GetProperty( name );
    }

    return GetProperty( name, defaultval );
}

std::optional<std::nullptr_t>
JsWindow::NotifyOthers( const std::wstring& name, JS::HandleValue info )
{
    std::wstring jsonStr;
    auto jsonCopyFunc = []( const char16_t* buf, uint32_t len, void* data )
    {
        assert( data );
        std::wstring* pJsonStr = (std::wstring*)data;
        pJsonStr->assign((const wchar_t*)buf, len );
        return true;
    };
    
    if ( info.isObject() )
    {
        JS::RootedObject jsObject( pJsCtx_, &info.toObject() );
        if ( !JS::ToJSONMaybeSafely( pJsCtx_, jsObject, jsonCopyFunc, &jsonStr ) )
        {
            JS_ReportErrorUTF8( pJsCtx_, "Unsuitable info argument" );
            return std::nullopt;
        }
    }
    else if (info.isPrimitive() && !info.isNullOrUndefined())
    {
        JS::RootedValue valueCopy( pJsCtx_, info );
        if ( !JS_Stringify( pJsCtx_, &valueCopy, nullptr, JS::NullHandleValue, jsonCopyFunc, &jsonStr ) )
        {
            JS_ReportErrorUTF8( pJsCtx_, "Unsuitable info argument" );
            return std::nullopt;
        }
    }
    else
    {
        JS_ReportErrorUTF8( pJsCtx_, "Unsuitable info argument" );
        return std::nullopt;
    }

    // TODO: think about replacing with PostMessage
    panel_manager::instance().send_msg_to_others_pointer( parentPanel_.GetHWND(), CALLBACK_UWM_ON_NOTIFY_DATA, 
                                                          std::make_unique<std::tuple<std::wstring, std::wstring>>(name, jsonStr));
    
    return nullptr;
}

std::optional<std::nullptr_t>
JsWindow::Reload()
{
    PostMessage( parentPanel_.GetHWND(), UWM_RELOAD, 0, 0 );
    return nullptr;
}

std::optional<std::nullptr_t>
JsWindow::Repaint( bool force )
{
    parentPanel_.Repaint( force );
    return nullptr;
}

std::optional<std::nullptr_t> JsWindow::RepaintWithOpt( size_t optArgCount, bool force )
{
    if ( optArgCount > 1 )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Internal error: invalid number of optional arguments specified: %d", optArgCount );
        return std::nullopt;
    }

    if ( optArgCount == 1 )
    {
        return Repaint();
    }

    return Repaint( force );
}

std::optional<std::nullptr_t>
JsWindow::RepaintRect( uint32_t x, uint32_t y, uint32_t w, uint32_t h, bool force )
{
    parentPanel_.RepaintRect( x, y, w, h, force );
    return nullptr;
}

std::optional<std::nullptr_t> JsWindow::RepaintRectWithOpt( size_t optArgCount, uint32_t x, uint32_t y, uint32_t w, uint32_t h, bool force )
{
    if ( optArgCount > 1 )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Internal error: invalid number of optional arguments specified: %d", optArgCount );
        return std::nullopt;
    }

    if ( optArgCount == 1 )
    {
        return RepaintRect( x, y, w, h );
    }

    return RepaintRect( x, y, w, h, force );
}

std::optional<std::nullptr_t>
JsWindow::SetCursor( uint32_t id )
{
    ::SetCursor( LoadCursor( nullptr, MAKEINTRESOURCE( id ) ) );
    return nullptr;
}

std::optional<uint32_t>
JsWindow::SetInterval( JS::HandleValue func, uint32_t delay )
{// TODO: try to remove the roundabout call (JsWindow > js_panel_window > JsContainer)
    if ( !func.isObject() || !JS_ObjectIsFunction( pJsCtx_, &func.toObject() ) )
    {
        JS_ReportErrorUTF8( pJsCtx_, "func argument is not a JS function" );
        return std::nullopt;
    }

    JS::RootedFunction jsFunction( pJsCtx_, JS_ValueToFunction( pJsCtx_, func ) );
    return parentPanel_.SetInterval( jsFunction, delay );
}

std::optional<std::nullptr_t>
JsWindow::SetProperty( const std::wstring& name, JS::HandleValue val )
{
    if ( !fbProperties_->SetProperty( name, val ) )
    {// report in SetProperty
        return std::nullopt;
    }

    return nullptr;
}

std::optional<std::nullptr_t> 
JsWindow::SetPropertyWithOpt( size_t optArgCount, const std::wstring& name, JS::HandleValue val )
{
    if ( optArgCount > 1 )
    {
        JS_ReportErrorUTF8( pJsCtx_, "Internal error: invalid number of optional arguments specified: %d", optArgCount );
        return std::nullopt;
    }

    if ( optArgCount == 1 )
    {
        return SetProperty( name );
    }

    return SetProperty( name, val );
}

std::optional<uint32_t>
JsWindow::SetTimeout( JS::HandleValue func, uint32_t delay )
{    
    if ( !func.isObject() || !JS_ObjectIsFunction( pJsCtx_, &func.toObject() ) )
    {
        JS_ReportErrorUTF8( pJsCtx_, "func argument is not a JS function" );
        return std::nullopt;
    }

    JS::RootedFunction jsFunction( pJsCtx_, JS_ValueToFunction( pJsCtx_, func ) );
    return parentPanel_.SetTimeout( jsFunction, delay );
}

std::optional<std::nullptr_t>
JsWindow::ShowConfigure()
{
    PostMessage( parentPanel_.GetHWND(), UWM_SHOW_CONFIGURE, 0, 0 );
    return nullptr;
}

std::optional<std::nullptr_t>
JsWindow::ShowProperties()
{
    PostMessage( parentPanel_.GetHWND(), UWM_SHOW_PROPERTIES, 0, 0 );
    return nullptr;
}

std::optional<uint32_t>
JsWindow::get_DlgCode()
{
    return parentPanel_.DlgCode();
}

std::optional<uint32_t>
JsWindow::get_Height()
{
    return parentPanel_.GetHeight();
}

std::optional<uint32_t>
JsWindow::get_ID()
{// Will work properly only on x86
    return reinterpret_cast<uint32_t>( parentPanel_.GetHWND() );
}

std::optional<uint32_t>
JsWindow::get_InstanceType()
{
    return static_cast<uint32_t>( parentPanel_.GetPanelType() );
}

std::optional<bool>
JsWindow::get_IsTransparent()
{
    return parentPanel_.get_pseudo_transparent();
}

std::optional<bool>
JsWindow::get_IsVisible()
{
    return  IsWindowVisible( parentPanel_.GetHWND() );
}

std::optional<uint32_t>
JsWindow::get_MaxHeight()
{
    return parentPanel_.MaxSize().y;
}

std::optional<uint32_t>
JsWindow::get_MaxWidth()
{
    return parentPanel_.MaxSize().x;
}

std::optional<uint32_t>
JsWindow::get_MinHeight()
{
    return  parentPanel_.MinSize().y;
}

std::optional<uint32_t>
JsWindow::get_MinWidth()
{
    return parentPanel_.MinSize().x;
}

std::optional<pfc::string8_fast>
JsWindow::get_Name()
{
    pfc::string8_fast name = parentPanel_.ScriptInfo().name;
    if ( name.is_empty() )
    {
        name = pfc::print_guid( parentPanel_.GetGUID() );
    }

    return pfc::string8_fast( name.c_str(), name.length() );
}

std::optional<uint32_t>
JsWindow::get_Width()
{
    return parentPanel_.GetWidth();
}

std::optional<std::nullptr_t>
JsWindow::put_DlgCode( uint32_t code )
{
    parentPanel_.DlgCode() = code;
    return nullptr;
}

std::optional<std::nullptr_t>
JsWindow::put_MaxHeight( uint32_t height )
{
    parentPanel_.MaxSize().y = height;
    PostMessage( parentPanel_.GetHWND(), UWM_SIZE_LIMIT_CHANGED, 0, uie::size_limit_maximum_height );
    return nullptr;
}

std::optional<std::nullptr_t>
JsWindow::put_MaxWidth( uint32_t width )
{
    parentPanel_.MaxSize().x = width;
    PostMessage( parentPanel_.GetHWND(), UWM_SIZE_LIMIT_CHANGED, 0, uie::size_limit_maximum_width );
    return nullptr;
}

std::optional<std::nullptr_t>
JsWindow::put_MinHeight( uint32_t height )
{
    parentPanel_.MinSize().y = height;
    PostMessage( parentPanel_.GetHWND(), UWM_SIZE_LIMIT_CHANGED, 0, uie::size_limit_minimum_height );
    return nullptr;
}

std::optional<std::nullptr_t>
JsWindow::put_MinWidth( uint32_t width )
{
    parentPanel_.MinSize().x = width;
    PostMessage( parentPanel_.GetHWND(), UWM_SIZE_LIMIT_CHANGED, 0, uie::size_limit_minimum_width );
    return nullptr;
}

}
