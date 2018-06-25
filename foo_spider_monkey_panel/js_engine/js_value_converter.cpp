#include <stdafx.h>

#include "js_value_converter.h"
#include <js_objects/gdi_font.h>

namespace
{

template <typename NativeObjectType>
bool JsToNativeObject( JSContext * cx, const JS::HandleValue& jsValue, NativeObjectType*& unwrappedValue )
{
    if ( !jsValue.isObjectOrNull() )
    {
        return false;
    }

    JS::RootedObject jsObject( cx, jsValue.toObjectOrNull() );
    if ( !jsObject )
    {
        unwrappedValue = NULL;
        return true;
    }

    if ( !JS_InstanceOf( cx, jsObject, &NativeObjectType::GetClass(), nullptr ) )
    {
        return false;
    }

    auto pJsFont = static_cast<NativeObjectType *>(JS_GetPrivate( jsObject ));
    if ( !pJsFont )
    {
        assert( 0 );
        return false;
    }

    unwrappedValue = pJsFont;
    return true;
}

}

namespace mozjs
{

bool NativeToJsValue( JSContext * cx, JS::HandleObject inValue, JS::MutableHandleValue wrappedValue )
{
    wrappedValue.setObjectOrNull( inValue );
    return true;
}

template <>
bool NativeToJsValue<bool>( JSContext *, const bool& inValue, JS::MutableHandleValue wrappedValue )
{
    wrappedValue.setBoolean( inValue );
    return true;
}

template <>
bool NativeToJsValue<int32_t>( JSContext *, const int32_t& inValue, JS::MutableHandleValue wrappedValue )
{
    wrappedValue.setInt32( inValue );
    return true;
}

template <>
bool NativeToJsValue<uint32_t>( JSContext *, const uint32_t& inValue, JS::MutableHandleValue wrappedValue )
{
    wrappedValue.setNumber( inValue );
    return true;
}

template <>
bool NativeToJsValue<double>( JSContext *, const double& inValue, JS::MutableHandleValue wrappedValue )
{
    wrappedValue.setNumber( inValue );
    return true;
}

template <>
bool NativeToJsValue<float>( JSContext *, const float& inValue, JS::MutableHandleValue wrappedValue )
{
    wrappedValue.setNumber( inValue );
    return true;
}

template <>
bool NativeToJsValue<std::string_view>( JSContext * cx, const std::string_view& inValue, JS::MutableHandleValue wrappedValue )
{
    JSString* jsString = JS_NewStringCopyZ( cx, inValue.data() );
    if ( !jsString )
    {
        return false;
    }

    wrappedValue.setString( jsString );
    return true;
}

template <>
bool NativeToJsValue<std::wstring_view>( JSContext * cx, const std::wstring_view& inValue, JS::MutableHandleValue wrappedValue )
{
    // <codecvt> is deprecated in C++17...
    std::string tmpString (pfc::stringcvt::string_utf8_from_wide( inValue.data() ));
    return NativeToJsValue<std::string_view>( cx, tmpString, wrappedValue );
}

template <>
bool NativeToJsValue<std::wstring>( JSContext * cx, const std::wstring& inValue, JS::MutableHandleValue wrappedValue )
{
    return NativeToJsValue<std::wstring_view>( cx, inValue, wrappedValue );
}


template <>
bool NativeToJsValue<std::nullptr_t>( JSContext *, const std::nullptr_t& inValue, JS::MutableHandleValue wrappedValue )
{
    wrappedValue.setUndefined();
    return true;
}

bool JsToNative<bool>::IsValid( JSContext * cx, const JS::HandleValue& jsValue )
{
    return jsValue.isBoolean();
}
bool JsToNative<bool>::Convert( JSContext * cx, const JS::HandleValue& jsValue )
{
    return std::forward<bool>( jsValue.toBoolean() );
}

bool JsToNative<int32_t>::IsValid( JSContext * cx, const JS::HandleValue& jsValue )
{
    return jsValue.isInt32();
}
int32_t JsToNative<int32_t>::Convert( JSContext * cx, const JS::HandleValue& jsValue )
{
    return std::forward<int32_t>( jsValue.isInt32() );
}

bool JsToNative<uint32_t>::IsValid( JSContext * cx, const JS::HandleValue& jsValue )
{
    return jsValue.isNumber();
}
uint32_t JsToNative<uint32_t>::Convert( JSContext * cx, const JS::HandleValue& jsValue )
{
    return std::forward<uint32_t>( static_cast<uint32_t>( jsValue.toNumber() ) );
}

bool JsToNative<float>::IsValid( JSContext * cx, const JS::HandleValue& jsValue )
{
    return jsValue.isNumber();
}
float JsToNative<float>::Convert( JSContext * cx, const JS::HandleValue& jsValue )
{
    return std::forward<float>( static_cast<float>( jsValue.toNumber() ) );
}

bool JsToNative<double>::IsValid( JSContext * cx, const JS::HandleValue& jsValue )
{
    return jsValue.isNumber();
}
double JsToNative<double>::Convert( JSContext * cx, const JS::HandleValue& jsValue )
{
    return std::forward<double>( jsValue.toNumber() );
}

bool JsToNative<std::string>::IsValid( JSContext * cx, const JS::HandleValue& jsValue )
{
    return jsValue.isString();
}
std::string JsToNative<std::string>::Convert( JSContext * cx, const JS::HandleValue& jsValue )
{
    JS::RootedString jsString( cx, jsValue.toString() );
    std::unique_ptr<
        const char, std::function<void( const char* )>
    > encodedString(
        JS_EncodeStringToUTF8( cx, jsString ), 
        [&]( const char* str )
        {
            JS_free( cx, (void*)str );;
        }
    );

    return std::forward<std::string>( std::string( encodedString.get()) );
}

bool JsToNative<std::wstring>::IsValid( JSContext * cx, const JS::HandleValue& jsValue )
{
    return jsValue.isString();
}
std::wstring JsToNative<std::wstring>::Convert( JSContext * cx, const JS::HandleValue& jsValue )
{
    std::string tmpString( JsToNative<std::string>::Convert( cx, jsValue ) );

    // <codecvt> is deprecated in C++17...
    return std::forward<std::wstring>( std::wstring( pfc::stringcvt::string_wide_from_utf8( tmpString.c_str() ) ) );
}

bool JsToNative<std::nullptr_t>::IsValid( JSContext * cx, const JS::HandleValue& jsValue )
{
    return true;
}
std::nullptr_t JsToNative<std::nullptr_t>::Convert( JSContext * cx, const JS::HandleValue& jsValue )
{
    return nullptr;
}

}
