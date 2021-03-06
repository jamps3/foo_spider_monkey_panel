#pragma once

#include <js_objects/object_base.h>

#include <optional>

class JSObject;
struct JSContext;
struct JSClass;

namespace mozjs
{

class JsFbFileInfo
    : public JsObjectBase<JsFbFileInfo>
{
public:
    static constexpr bool HasProto = true;
    static constexpr bool HasGlobalProto = false;
    static constexpr bool HasProxy = false;
    static constexpr bool HasPostCreate = false;

    static const JSClass JsClass;
    static const JSFunctionSpec* JsFunctions;
    static const JSPropertySpec* JsProperties;
    static const JsPrototypeId PrototypeId;

public:
    ~JsFbFileInfo();

    static std::unique_ptr<JsFbFileInfo> CreateNative( JSContext* cx, std::unique_ptr<file_info_impl> fileInfo );
    static size_t GetInternalSize( const std::unique_ptr<file_info_impl>& fileInfo );

public:
    int32_t InfoFind( const pfc::string8_fast& name );
    pfc::string8_fast InfoName( uint32_t idx );
    pfc::string8_fast InfoValue( uint32_t idx );
    int32_t MetaFind( const pfc::string8_fast& name );
    pfc::string8_fast MetaName( uint32_t idx );
    pfc::string8_fast MetaValue( uint32_t idx, uint32_t vidx );
    uint32_t MetaValueCount( uint32_t idx );

public:
    uint32_t get_InfoCount();
    uint32_t get_MetaCount();

private:
    JsFbFileInfo( JSContext* cx, std::unique_ptr<file_info_impl> fileInfo );

private:
    JSContext* pJsCtx_ = nullptr;
    std::unique_ptr<file_info_impl> fileInfo_;
};

} // namespace mozjs
