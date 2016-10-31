#include "hparser.h"
#include "csocket.h"

namespace HARMONY {
	
class DocumentState : public HStateAdapter {
public:
    DocumentState(HWriter* writer) : HStateAdapter("Document", writer) {}
};
    
class ConnectState : public HStateAdapter {
public:
    ConnectState() : HStateAdapter("stream:stream") {}

protected:
    ERROR on_start_element(const Glib::ustring& name,
						   const xmlpp::SaxParser::AttributeList& attributes) override {
        return ERROR::E_SUCCESS;
    }

    ERROR on_end_element(const Glib::ustring& name) override {
        return ERROR::E_SUCCESS;
    }
};

class ConnectError : public HStateAdapter {
public:
    ConnectError(HWriter* writer) : HStateAdapter("stream:error", writer) {}
};

class StreamFeaturesState : public HStateAdapter {
public:
    StreamFeaturesState(HWriter* writer) : HStateAdapter("stream:features", writer) {}
};

class MechanismsState : public HStateAdapter {
public:
    MechanismsState() : HStateAdapter("mechanisms") {}
public:
};

class MechanismState : public HStateAdapter {
public:
    MechanismState() : HStateAdapter("mechanism") {}
};

class SuccessState : public HStateAdapter {
public:
    SuccessState(HWriter* writer) : HStateAdapter("success", writer) {}
};

class IqState : public HStateAdapter {
public:
    IqState(HWriter* writer = 0) : HStateAdapter("iq", writer) {}
};

class OaSwapTokenState : public HStateAdapter {
public:
    OaSwapTokenState() : HStateAdapter("oa") {}
    
protected:
    ERROR on_cdata_block (const Glib::ustring& text) override;
    ERROR on_start_element(const Glib::ustring& name,
                           const xmlpp::SaxParser::AttributeList& attributes) override;	
	
private:
	bool isPing;
	bool isPair;
};

class MessageState : public HStateAdapter {
public:
    MessageState() : HStateAdapter("message") {}
};

class EventState : public HStateAdapter {
public:
    EventState() : HStateAdapter("event") {}
    
    ERROR on_cdata_block (const Glib::ustring& text) override;
};

} // END namespace
