TEMPLATE = subdirs

# LipidSpace unit tests (QtTest). Run all suites with:
#     qmake6 tests/tests.pro && make check
# Each subdir is a self-contained test executable that compiles the relevant
# production sources directly (no dependency on the full app/REST build).
SUBDIRS += \
    tst_matrix_blas
