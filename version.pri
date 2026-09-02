# ---------------------------------------------------------------------------
# Version - shared by LipidSpace.pro and LipidSpaceRest.pro.
#
# Single source of truth, in this order of precedence:
#   1. $RELEASE_VERSION  - set by .github/workflows/release.yml from the git tag,
#                          and by LipidSpaceRest.docker from its build argument
#   2. git describe      - e.g. "v1.2.1-53-g59779b40" for local/dev builds
#   3. LIPIDSPACE_FALLBACK_VERSION below, for builds from a source tarball or
#                          from a container image with no git available
#
# Reaches the code as the LIPIDSPACE_VERSION macro; see src/globaldata.cpp.
# Do not hard-code the version anywhere else.
# ---------------------------------------------------------------------------
LIPIDSPACE_FALLBACK_VERSION = v1.2.1

LIPIDSPACE_VERSION = $$(RELEASE_VERSION)
isEmpty(LIPIDSPACE_VERSION) {
    exists($$PWD/.git) {
        LIPIDSPACE_VERSION = $$system(git -C $$shell_quote($$PWD) describe --tags --always --dirty)
    }
}
isEmpty(LIPIDSPACE_VERSION): LIPIDSPACE_VERSION = $$LIPIDSPACE_FALLBACK_VERSION
DEFINES += LIPIDSPACE_VERSION=\\\"$$LIPIDSPACE_VERSION\\\"

# Numeric x.y.z form for the Windows .exe / macOS bundle version resource.
# "v1.2.1-53-g59779b40" -> "1.2.1". Left unset if nothing numeric can be derived.
LIPIDSPACE_VERSION_NUMERIC = $$replace(LIPIDSPACE_VERSION, "^[^0-9]*", "")
LIPIDSPACE_VERSION_NUMERIC = $$replace(LIPIDSPACE_VERSION_NUMERIC, "[^0-9.].*$", "")
!isEmpty(LIPIDSPACE_VERSION_NUMERIC): VERSION = $$LIPIDSPACE_VERSION_NUMERIC

# !build_pass so this is printed once, not once per debug/release pass.
!build_pass: message("LipidSpace version: $$LIPIDSPACE_VERSION")
