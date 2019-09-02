#include "expression_test_logger.hpp"
#include "expression_test_runner.hpp"
#include "filesystem.hpp"

#include <mbgl/util/io.hpp>
#include <mbgl/util/string.hpp>

using namespace mbgl;
using namespace std::literals;

namespace {

const char* resultsStyle = R"HTML(
<style>
    body { font: 18px/1.2 -apple-system, BlinkMacSystemFont, "Helvetica Neue", Helvetica, Arial, sans-serif; padding: 10px; }
    h1 { font-size: 32px; margin-bottom: 0; }
    button { vertical-align: middle; }
    h2 { font-size: 24px; font-weight: normal; margin: 10px 0 10px; line-height: 1; }
    .test { border-bottom: 1px dotted #bbb; padding-bottom: 5px; }
    .tests { border-top: 1px dotted #bbb; margin-top: 10px; }
    .test p, .test pre { margin: 0 0 10px; }
    .test pre { font-size: 14px; }
    .label { color: white; font-size: 18px; padding: 2px 6px 3px; border-radius: 3px; margin-right: 3px; vertical-align: bottom; display: inline-block; }
    .hide { display: none; }
    .test.failed > h2 > span { background: red; }
    .test.passed > h2 > span { background: green; }
    .test.ignored > h2 > span { background: grey; }
    .test.errored > h2 > span { background: #ff006f; }
    .test.ignored.passed > h2 > span { background: #E8A408; }
</style>
)HTML";

const char* resultsHeaderButtons = R"HTML(
    <button id='toggle-passed'>Toggle passed tests</button>
    <button id='toggle-ignored'>Toggle ignored tests</button>
</h1>
)HTML";

const char* resultsScript = R"HTML(
<script>
document.getElementById('toggle-passed').addEventListener('click', function (e) {
    for (const row of document.querySelectorAll('.test.passed')) {
        row.classList.toggle('hide');
    }
});
document.getElementById('toggle-ignored').addEventListener('click', function (e) {
    for (const row of document.querySelectorAll('.test.ignored')) {
        row.classList.toggle('hide');
    }
});
</script>
)HTML";

std::string createResultItem(const TestRunOutput& result, const std::string& status, bool shouldHide) {
    std::string html;
    html.append("<div class=\"test " + status + (shouldHide ? " hide" : "") + "\">\n");
    html.append(R"(<h2><span class="label">)"  + status + "</span> " + result.id + "</h2>\n");

    html.append("<p><pre>"s + result.expression + "</pre></p>\n"s);
    if (result.passed) {
        html.append("<strong>Serialized:</strong><p><pre>"s + result.serialized + "</pre></p>\n"s);
    } else {
        html.append("<p><strong>Difference:</strong><pre>" + result.text + "</pre></p>\n");
    }
    return html.append("</div>\n");
}

std::string createResultPage(const TestStats& stats) {
    const std::size_t unsuccessful = stats.errored.size() + stats.failed.size();
    std::string resultsPage;
    // Style
    resultsPage.append(resultsStyle);

    // Header with buttons
    if (unsuccessful) {
        resultsPage.append(R"HTML(<h1 style="color: red;">)HTML");
        resultsPage.append(util::toString(unsuccessful) + " tests failed.");
    } else {
        resultsPage.append(R"HTML(<h1 style="color: green;">)HTML");
        resultsPage.append("All tests passed!");
    }
    resultsPage.append(resultsHeaderButtons);

    // Script
    resultsPage.append(resultsScript);

     // Tests
    resultsPage.append("<div class=\"tests\">\n");
    const auto appendResult = [&] (const auto& results, const std::string& status, bool hide = false) {
        for (const auto& result : results) {
            resultsPage.append(createResultItem(result, status, hide));
        }
    };

    appendResult(stats.passed, "passed"s, bool(unsuccessful));
    appendResult(stats.failed, "failed"s);
    appendResult(stats.errored, "errored"s);
    appendResult(stats.ignorePassed, "ignored passed"s, bool(unsuccessful));
    appendResult(stats.ignoreFailed, "ignored"s, true);
    return resultsPage.append("</div>\n");
}

} // namespace

void printStats(const TestStats& stats) {
    const std::size_t count = stats.testCount();
    if (std::size_t passedTests = stats.passed.size()) {
        printf(ANSI_COLOR_GREEN "%zu passed (%.1lf%%)" ANSI_COLOR_RESET "\n", passedTests, 100.0 * passedTests / count);
    }
    if (std::size_t ignorePassedTests = stats.ignorePassed.size()) {
        printf(ANSI_COLOR_YELLOW "%zu passed but were ignored (%.1lf%%)" ANSI_COLOR_RESET "\n", ignorePassedTests, 100.0 * ignorePassedTests / count);
    }
    if (std::size_t ignoreFailedTests = stats.ignoreFailed.size()) {
        printf(ANSI_COLOR_LIGHT_GRAY "%zu ignored (%.1lf%%)" ANSI_COLOR_RESET "\n", ignoreFailedTests, 100.0 * ignoreFailedTests / count);
    }
    if (std::size_t failedTests = stats.failed.size()) {
        printf(ANSI_COLOR_RED "%zu failed (%.1lf%%)" ANSI_COLOR_RESET "\n", failedTests, 100.0 * failedTests / count);
    }
    if (std::size_t erroredTests = stats.errored.size()) {
        printf(ANSI_COLOR_RED "%zu errored (%.1lf%%)" ANSI_COLOR_RESET "\n", erroredTests, 100.0 * erroredTests / count);
    }
}

void writeHTMLResults(const TestStats& stats, const std::string& rootPath) {
    filesystem::path path = filesystem::path(rootPath) / "index.html"s;
    try {
        util::write_file(path.string(), createResultPage(stats));
        printf("Results at: %s\n", path.string().c_str());
    } catch (std::exception&) {
        printf(ANSI_COLOR_RED "* ERROR can't write result page %s" ANSI_COLOR_RESET "\n", path.string().c_str());
    }
}
