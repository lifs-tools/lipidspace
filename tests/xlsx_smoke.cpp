// Guards the vendored OpenXLSX against header/binary drift.
//
// OpenXLSX's IZipArchive is a header-only, type-erased interface: the vtable
// layout comes from libraries/OpenXLSX/include/headers/IZipArchive.hpp, but the
// calls through it are compiled into libOpenXLSX.a. If the headers and the
// archive come from different upstream revisions, the build still links cleanly
// and then dispatches through a wrong vtable slot at runtime — every XLSX import
// segfaults with no error message, which is exactly what shipped on Windows.
//
// The linker cannot catch that, so this does. Build and run it on every platform.
//
//   g++ -std=c++17 -I libraries/OpenXLSX/include tests/xlsx_smoke.cpp \
//       libraries/OpenXLSX/bin/<platform>/libOpenXLSX.a -o xlsx_smoke
//   ./xlsx_smoke examples/Example-Dataset.xlsx
//
// See libraries/OpenXLSX/PROVENANCE.md.

#include <OpenXLSX.hpp>

#include <iostream>
#include <string>

using namespace OpenXLSX;

namespace {

// Example-Dataset.xlsx: a header row plus 32 lipidome rows, and the 369 columns
// that LipidSpaceGUI::openExampleDataset() hard-codes.
constexpr int EXPECTED_ROWS    = 33;
constexpr int EXPECTED_COLUMNS = 369;

int fail(const std::string& message)
{
    std::cerr << "FAIL: " << message << std::endl;
    return 1;
}

}    // namespace

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: xlsx_smoke <path-to-Example-Dataset.xlsx>" << std::endl;
        return 2;
    }

    const std::string path { argv[1] };

    try {
        XLDocument doc { path };
        auto       wks = doc.workbook().worksheet("Data");

        const int rows    = static_cast<int>(wks.rowCount());
        const int columns = static_cast<int>(wks.columnCount());

        if (rows != EXPECTED_ROWS) {
            return fail("expected " + std::to_string(EXPECTED_ROWS) + " rows, got " + std::to_string(rows));
        }
        if (columns != EXPECTED_COLUMNS) {
            return fail("expected " + std::to_string(EXPECTED_COLUMNS) + " columns, got " + std::to_string(columns));
        }

        // Iterating rows and cells is what FileTableHandler does, and it is the
        // path that dispatches through IZipArchive. Reading the values matters:
        // a mismatched build can survive opening the document and only fall over
        // once the shared-strings table is touched.
        int seen = 0;
        for (auto& row : wks.rows()) {
            int cells = 0;
            for (auto cell : row.cells()) {
                (void)cell.value().typeAsString();
                ++cells;
            }
            if (cells != EXPECTED_COLUMNS) {
                return fail("row " + std::to_string(seen) + " has " + std::to_string(cells) + " cells, expected "
                            + std::to_string(EXPECTED_COLUMNS));
            }
            ++seen;
        }

        if (seen != EXPECTED_ROWS) {
            return fail("iterated " + std::to_string(seen) + " rows, expected " + std::to_string(EXPECTED_ROWS));
        }

        doc.close();
    }
    catch (const std::exception& e) {
        return fail(std::string { "exception: " } + e.what());
    }
    catch (...) {
        return fail("unknown exception");
    }

    std::cout << "OK: read " << EXPECTED_ROWS << " rows x " << EXPECTED_COLUMNS << " columns from " << path << std::endl;
    return 0;
}
