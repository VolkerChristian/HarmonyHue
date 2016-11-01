#include <glibmm/convert.h>

#include "hparser.h"
#include "logger.h"

namespace HARMONY {

void HParser::p_on_start_document() {
    LogDebug << "on_start_document()";
}

void HParser::p_on_end_document() {
    LogDebug << "on_end_document()";
}

void HParser::p_on_start_element(const Glib::ustring& name,
                               const AttributeList& attributes) {
    LogDebug << "on_start_element(): " << name;

    // Print attributes:
    for(const auto& attr_pair : attributes)
    {
        try
        {
            LogDebug << "  Attribute: " <<  attr_pair.name << " : " << attr_pair.value;
        }
        catch(const Glib::ConvertError& ex)
        {
            LogWarn << "MySaxParser::on_start_element(): Exception caught while converting name for LogDebug: " << ex.what();
        }
    }
}

void HParser::p_on_end_element(const Glib::ustring& name) {
    LogDebug << "on_end_element(): " << name;
}

void HParser::p_on_characters(const Glib::ustring& characters) {
    try
    {
        LogDebug << "on_characters(): " << characters;
    }
    catch(const Glib::ConvertError& ex)
    {
        LogWarn << "MySaxParser::on_characters(): Exception caught while converting text for LogDebug: " << ex.what();
    }
}

void HParser::p_on_comment(const Glib::ustring& text) {
    try
    {
        LogDebug << "on_comment(): " << text;
    }
    catch(const Glib::ConvertError& ex)
    {
        LogWarn << "MySaxParser::on_comment(): Exception caught while converting text for LogDebug: " << ex.what();
    }
}

void HParser::p_on_warning(const Glib::ustring& text) {
    try
    {
        LogDebug << "on_warning(): " << text;
    }
    catch(const Glib::ConvertError& ex)
    {
        LogWarn << "MySaxParser::on_warning(): Exception caught while converting text for LogDebug: " << ex.what();
    }
}

void HParser::p_on_error(const Glib::ustring& text) {
    try
    {
        LogDebug << "on_error(): " << text;
    }
    catch(const Glib::ConvertError& ex)
    {
        LogWarn << "MySaxParser::on_error(): Exception caught while converting text for LogDebug: " << ex.what();
    }
}

void HParser::p_on_fatal_error(const Glib::ustring& text) {
    try
    {
        LogDebug << "on_fatal_error(): " << text;
    }
    catch(const Glib::ConvertError& ex)
    {
        LogWarn << "MySaxParser::on_characters(): Exception caught while converting value for LogDebug: " << ex.what();
    }
}

void HParser::p_on_cdata_block (const Glib::ustring& text) {
    try
    {
        LogDebug << "on_cdata_block(): " << text;
    }
    catch(const Glib::ConvertError& ex)
    {
        LogWarn << "MySaxParser::on_characters(): Exception caught while converting text for LogDebug: " << ex.what();
    }
}

} // END namespace
