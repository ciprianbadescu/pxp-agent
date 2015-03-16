#include "test/test.h"

#include "src/errors.h"
#include "src/modules/echo.h"

#include <cthun-client/data_container/data_container.h>
#include <cthun-client/message/chunks.h>       // ParsedChunks

#include <string>
#include <vector>

namespace CthunAgent {

// TODO(ale): update this to cthun-client

static const std::string echo_action { "echo" };
static const std::string fake_action { "fake_action" };

static const std::string echo_data_txt {
    "{  \"module\" : \"echo\","
    "   \"action\" : \"echo\","
    "   \"params\" : { \"txt\" : \"maradona\" }"
    "}"
};

static const std::vector<std::string> no_debug {};

static const CthunClient::ParsedChunks parsed_chunks {
                CthunClient::DataContainer(),
                CthunClient::DataContainer(echo_data_txt),
                no_debug };

static const std::string bad_echo_data_txt {
    "{  \"module\" : \"echo\","
    "   \"action\" : \"echo\","
    "   \"params\" : [1, 2, 3, 4 ,5]"
    "}"
};

static const CthunClient::ParsedChunks bad_parsed_chunks {
                CthunClient::DataContainer(),
                CthunClient::DataContainer(bad_echo_data_txt),
                no_debug };

TEST_CASE("Module::validateAndCallAction", "[modules]") {
    Modules::Echo echo_module {};

    SECTION("it should correctly call echo") {
        auto result = echo_module.performRequest(echo_action, parsed_chunks);
        auto txt = result.get<std::string>("txt");
        REQUIRE(txt == "maradona");
    }

    SECTION("it should throw a request_validation_error if the action "
            "is unknown") {
        REQUIRE_THROWS_AS(echo_module.performRequest(fake_action, parsed_chunks),
                          request_validation_error);
    }

    SECTION("it should throw a request_validation_error if the request "
            "is invalid") {
        REQUIRE_THROWS_AS(echo_module.performRequest(echo_action,
                                                     bad_parsed_chunks),
                          request_validation_error);
    }
}

}  // namespace CthunAgent
