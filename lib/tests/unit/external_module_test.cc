#include "root_path.hpp"

#include <cthun-agent/external_module.hpp>

#include <cthun-client/protocol/chunks.hpp>       // ParsedChunks

#include <leatherman/json_container/json_container.hpp>

#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>

#include <catch.hpp>

#include <string>
#include <vector>
#include <unistd.h>

namespace CthunAgent {

namespace lth_jc = leatherman::json_container;

const std::string RESULTS_ROOT_DIR { "/tmp/cthun-agent" };
const std::string STRING_ACTION { "string" };

boost::format data_format {
    "{  \"module\" : %1%,"
    "   \"action\" : %2%,"
    "   \"params\" : %3%"
    "}"
};

const std::string reverse_txt {
    (data_format % "\"reverse\""
                 % "\"string\""
                 % "{\"argument\" : \"maradona\"}").str() };

static const std::vector<lth_jc::JsonContainer> no_debug {};

static const CthunClient::ParsedChunks content {
                    lth_jc::JsonContainer(),             // envelope
                    lth_jc::JsonContainer(reverse_txt),  // data
                    no_debug,   // debug
                    0 };        // num invalid debug chunks

TEST_CASE("ExternalModule::ExternalModule", "[modules]") {
    SECTION("can successfully instantiate from a valid external module") {
        REQUIRE_NOTHROW(ExternalModule(std::string { CTHUN_AGENT_ROOT_PATH }
            + "/lib/tests/resources/modules/reverse_valid"));
    }

    SECTION("all actions are successfully loaded from a valid external module") {
        ExternalModule mod { std::string { CTHUN_AGENT_ROOT_PATH }
                             + "/lib/tests/resources/modules/failures_test" };
        REQUIRE(mod.actions.size() == 2);
    }

    SECTION("throw a Module::LoadingError in case the module has an invalid "
            "metadata schema") {
        REQUIRE_THROWS_AS(
            ExternalModule(std::string { CTHUN_AGENT_ROOT_PATH }
                           + "/lib/tests/resources/broken_modules/reverse_broken"),
            Module::LoadingError);
    }
}

TEST_CASE("ExternalModule::hasAction", "[modules]") {
    ExternalModule mod { std::string { CTHUN_AGENT_ROOT_PATH }
                         + "/lib/tests/resources/modules/reverse_valid" };

    SECTION("correctly reports false") {
        REQUIRE(!mod.hasAction("foo"));
    }

    SECTION("correctly reports true") {
        REQUIRE(mod.hasAction("string"));
    }
}

TEST_CASE("ExternalModule::callAction - blocking", "[modules]") {
    SECTION("the shipped 'reverse' module works correctly") {
        ExternalModule reverse_module { std::string { CTHUN_AGENT_ROOT_PATH }
                                        + "/modules/reverse" };

        SECTION("correctly call the shipped reverse module") {
            ActionRequest request { RequestType::Blocking, content };
            auto outcome = reverse_module.executeAction(request);

            REQUIRE(outcome.stdout.find("anodaram") != std::string::npos);
        }
    }

    SECTION("it should handle module failures") {
        ExternalModule test_reverse_module {
            std::string { CTHUN_AGENT_ROOT_PATH }
            + "/lib/tests/resources/modules/failures_test" };

        SECTION("throw a Module::ProcessingError if the module returns an "
                "invalid result") {
            std::string failure_txt { (data_format % "\"failures_test\""
                                                   % "\"get_an_invalid_result\""
                                                   % "\"maradona\"").str() };
            CthunClient::ParsedChunks failure_content {
                    lth_jc::JsonContainer(),
                    lth_jc::JsonContainer(failure_txt),
                    no_debug,
                    0 };
            ActionRequest request { RequestType::Blocking, failure_content };

            REQUIRE_THROWS_AS(test_reverse_module.executeAction(request),
                              Module::ProcessingError);
        }

        SECTION("throw a Module::ProcessingError if a blocking action throws "
                "an exception") {
            std::string failure_txt { (data_format % "\"failures_test\""
                                                   % "\"broken_action\""
                                                   % "\"maradona\"").str() };
            CthunClient::ParsedChunks failure_content {
                    lth_jc::JsonContainer(),
                    lth_jc::JsonContainer(failure_txt),
                    no_debug,
                    0 };
            ActionRequest request { RequestType::Blocking, failure_content };

            REQUIRE_THROWS_AS(test_reverse_module.executeAction(request),
                              Module::ProcessingError);
        }
    }
}

}  // namespace CthunAgent
