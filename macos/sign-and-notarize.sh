#!/usr/bin/env bash
#
# Sign, notarize, staple and package LipidSpace.app for distribution outside
# the Mac App Store (Developer ID).
#
# One-time prerequisites (see README.md, "Signing and notarization"):
#   * A "Developer ID Application" certificate in a keychain.
#   * Notarization credentials, either
#       - a notarytool keychain profile:   NOTARY_PROFILE=lipidspace-notary
#       - or an App Store Connect API key: ASC_KEY_PATH, ASC_KEY_ID, ASC_ISSUER_ID
#
# Usage:
#   macos/sign-and-notarize.sh [options]
#
# Options:
#   --app PATH           app bundle to process              (default: LipidSpace.app)
#   --deploy             run macdeployqt first to make the bundle self-contained
#   --identity NAME      signing identity  (default: the sole "Developer ID Application")
#   --entitlements PATH  entitlements plist for the main executable (default: none)
#   --dist-dir PATH      output directory                    (default: Build)
#   --dist-name NAME     base name for the artifacts   (default: LipidSpace-<version>)
#   --sign-only          sign and verify; skip notarization and packaging
#   --no-dmg             produce only a stapled .zip, no .dmg
#   -h, --help           show this help
#
# The flow is: deploy -> normalize install names -> check self-containment ->
# sign inside-out -> notarize the zipped app -> staple the app -> build the dmg
# -> notarize the dmg -> staple the dmg -> verify. The app is stapled *before*
# the dmg is built, so the ticket travels with the app when a user drags it out.

set -euo pipefail

APP="LipidSpace.app"
IDENTITY="${SIGN_IDENTITY:-}"
ENTITLEMENTS="${ENTITLEMENTS:-}"
DIST_DIR="Build"
DIST_NAME=""
RUN_DEPLOY=0
SIGN_ONLY=0
MAKE_DMG=1

NOTARY_PROFILE="${NOTARY_PROFILE:-}"
ASC_KEY_PATH="${ASC_KEY_PATH:-}"
ASC_KEY_ID="${ASC_KEY_ID:-}"
ASC_ISSUER_ID="${ASC_ISSUER_ID:-}"

MACHO_LIST=""
STAGE=""

cleanup() {
    [ -n "$MACHO_LIST" ] && rm -f "$MACHO_LIST"
    [ -n "$STAGE" ] && rm -rf "$STAGE"
    return 0
}
trap cleanup EXIT

die()  { printf '\nerror: %s\n' "$*" >&2; exit 1; }
step() { printf '\n==> %s\n' "$*"; }
info() { printf '    %s\n' "$*"; }

usage() { sed -n '3,/^set -euo/p' "$0" | sed 's/^#\{0,1\} \{0,1\}//;$d'; exit 0; }

while [ $# -gt 0 ]; do
    case "$1" in
        --app)          APP="$2";          shift 2 ;;
        --identity)     IDENTITY="$2";     shift 2 ;;
        --entitlements) ENTITLEMENTS="$2"; shift 2 ;;
        --dist-dir)     DIST_DIR="$2";     shift 2 ;;
        --dist-name)    DIST_NAME="$2";    shift 2 ;;
        --deploy)       RUN_DEPLOY=1;      shift ;;
        --sign-only)    SIGN_ONLY=1;       shift ;;
        --no-dmg)       MAKE_DMG=0;        shift ;;
        -h|--help)      usage ;;
        *)              die "unknown option: $1 (try --help)" ;;
    esac
done

# ---------------------------------------------------------------- preflight --
[ -d "$APP" ] || die "no such app bundle: $APP (build it with 'make release' first)"
command -v xcrun >/dev/null || die "xcrun not found; install the Xcode command line tools"
if [ -n "$ENTITLEMENTS" ] && [ ! -f "$ENTITLEMENTS" ]; then
    die "no such entitlements file: $ENTITLEMENTS"
fi

APP_NAME="$(basename "$APP" .app)"
PLIST="$APP/Contents/Info.plist"

if [ -z "$IDENTITY" ]; then
    # Use the Developer ID Application identity if there is exactly one.
    FOUND="$(security find-identity -v -p codesigning 2>/dev/null \
             | sed -n 's/.*"\(Developer ID Application: .*\)"/\1/p' || true)"
    COUNT="$(printf '%s' "$FOUND" | grep -c . || true)"
    if [ "$COUNT" -eq 0 ]; then
        die "no 'Developer ID Application' identity in the keychain.
       Create one in Xcode > Settings > Accounts > Manage Certificates > + ,
       or at https://developer.apple.com/account/resources/certificates .
       Then re-run, or pass --identity 'Developer ID Application: ... (TEAMID)'."
    fi
    if [ "$COUNT" -gt 1 ]; then
        die "several Developer ID Application identities found; pass --identity:
$FOUND"
    fi
    IDENTITY="$FOUND"
fi

VERSION="${VERSION:-}"
if [ -z "$VERSION" ]; then
    VERSION="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' "$PLIST" 2>/dev/null || true)"
fi
[ -n "$VERSION" ] || VERSION="0.0.0"
[ -n "$DIST_NAME" ] || DIST_NAME="$APP_NAME-$VERSION"

NOTARY_ARGS=()
if [ -n "$NOTARY_PROFILE" ]; then
    NOTARY_ARGS=(--keychain-profile "$NOTARY_PROFILE")
elif [ -n "$ASC_KEY_PATH" ] && [ -n "$ASC_KEY_ID" ] && [ -n "$ASC_ISSUER_ID" ]; then
    NOTARY_ARGS=(--key "$ASC_KEY_PATH" --key-id "$ASC_KEY_ID" --issuer "$ASC_ISSUER_ID")
elif [ "$SIGN_ONLY" -eq 0 ]; then
    die "no notarization credentials. Set NOTARY_PROFILE (see 'xcrun notarytool
       store-credentials'), or ASC_KEY_PATH + ASC_KEY_ID + ASC_ISSUER_ID for an
       App Store Connect API key. Use --sign-only to sign without notarizing."
fi

info "app       : $APP"
info "version   : $VERSION"
info "identity  : $IDENTITY"
info "artifacts : $DIST_DIR/$DIST_NAME.zip$([ "$MAKE_DMG" -eq 1 ] && printf ' and .dmg')"

# ------------------------------------------------------------------- deploy --
if [ "$RUN_DEPLOY" -eq 1 ]; then
    step "Running macdeployqt"
    command -v macdeployqt >/dev/null \
        || die "macdeployqt not on PATH (export PATH=\"\$(brew --prefix qt)/bin:\$PATH\")"
    macdeployqt "$APP" -verbose=1
fi

# ------------------------------------------- normalize bundled install names --
# macdeployqt rewrites what binaries *depend on*, but leaves some copied dylibs
# advertising their original Homebrew path as their own install ID. That is
# harmless at load time; rewriting it keeps build-machine paths out of shipped
# binaries and lets the check below be strict.
step "Normalizing install names of bundled libraries"
for lib in "$APP"/Contents/Frameworks/*.dylib; do
    [ -f "$lib" ] || continue
    id="$(otool -D "$lib" 2>/dev/null | tail -n +2 | head -1 || true)"
    case "$id" in
        @*|"") ;;
        *)
            info "$(basename "$lib"): $id -> @executable_path"
            install_name_tool -id "@executable_path/../Frameworks/$(basename "$lib")" "$lib"
            ;;
    esac
done

# --------------------------------------------------- self-containment check --
# A bundle that still links against /opt/homebrew or /usr/local runs on this
# machine and nowhere else. Notarization accepts such a bundle happily, so this
# has to be caught here.
step "Checking the bundle is self-contained"
MACHO_LIST="$(mktemp)"
find "$APP" -type f -print0 | xargs -0 file 2>/dev/null \
    | grep 'Mach-O' | sed 's/: *Mach-O.*//' > "$MACHO_LIST" || true
MACHO_COUNT="$(grep -c . "$MACHO_LIST" || true)"
[ "$MACHO_COUNT" -gt 0 ] || die "no Mach-O files found in $APP"

LEAKS=""
while IFS= read -r f; do
    [ -n "$f" ] || continue
    self_id="$(otool -D "$f" 2>/dev/null | tail -n +2 | head -1 || true)"
    deps="$(otool -L "$f" 2>/dev/null | tail -n +2 | awk '{print $1}' || true)"
    while IFS= read -r dep; do
        [ -n "$dep" ] || continue
        if [ "$dep" != "$self_id" ]; then
            case "$dep" in
                /opt/homebrew/*|/usr/local/*) LEAKS="$LEAKS
  $f -> $dep" ;;
            esac
        fi
    done <<DEPS
$deps
DEPS
done < "$MACHO_LIST"

if [ -n "$LEAKS" ]; then
    die "the bundle still depends on libraries outside it:$LEAKS
       Re-run with --deploy, or fix the dependency by hand."
fi
info "$MACHO_COUNT Mach-O files, no dependencies outside the bundle"

# --------------------------------------------------------------------- sign --
# Sign inside-out: nested code first, the bundle itself last. Hardened runtime
# (--options runtime) and a secure timestamp are both required for notarization.
step "Signing with hardened runtime"
SIGN_BASE=(--force --options runtime --timestamp --sign "$IDENTITY")

for fw in "$APP"/Contents/Frameworks/*.framework; do
    [ -d "$fw" ] || continue
    codesign "${SIGN_BASE[@]}" "$fw"
done

# Everything else that is Mach-O and not inside a framework: bundled dylibs, Qt
# plugins, helper tools. The main executable is covered by the bundle signature.
while IFS= read -r f; do
    [ -n "$f" ] || continue
    case "$f" in
        *.framework/*) continue ;;
        "$APP/Contents/MacOS/$APP_NAME") continue ;;
    esac
    codesign "${SIGN_BASE[@]}" "$f"
done < "$MACHO_LIST"

SIGN_APP=("${SIGN_BASE[@]}")
if [ -n "$ENTITLEMENTS" ]; then
    SIGN_APP+=(--entitlements "$ENTITLEMENTS")
fi
codesign "${SIGN_APP[@]}" "$APP"

step "Verifying the signature"
codesign --verify --deep --strict --verbose=2 "$APP"
codesign -dv --verbose=4 "$APP" 2>&1 | grep -E 'Authority|TeamIdentifier|Timestamp' || true

if [ "$SIGN_ONLY" -eq 1 ]; then
    step "Done (--sign-only): signed, not notarized"
    exit 0
fi

# ---------------------------------------------------------------- notarize ---
notarize() {
    target="$1"
    out="$(xcrun notarytool submit "$target" "${NOTARY_ARGS[@]}" --wait 2>&1 || true)"
    printf '%s\n' "$out"
    sub_id="$(printf '%s\n' "$out" | awk '/^ *id:/ {print $2; exit}' || true)"
    if ! printf '%s\n' "$out" | grep -q 'status: Accepted'; then
        if [ -n "$sub_id" ]; then
            printf '\n--- notarization log ---\n'
            xcrun notarytool log "$sub_id" "${NOTARY_ARGS[@]}" || true
        fi
        die "notarization of $(basename "$target") failed"
    fi
}

mkdir -p "$DIST_DIR"
ZIP="$DIST_DIR/$DIST_NAME.zip"
DMG="$DIST_DIR/$DIST_NAME.dmg"
UPLOAD_ZIP="$DIST_DIR/.$DIST_NAME-notarize.zip"

step "Submitting the app for notarization (usually a few minutes)"
rm -f "$UPLOAD_ZIP"
ditto -c -k --sequesterRsrc --keepParent "$APP" "$UPLOAD_ZIP"
notarize "$UPLOAD_ZIP"
rm -f "$UPLOAD_ZIP"

step "Stapling the ticket to the app"
xcrun stapler staple "$APP"

step "Packaging $ZIP"
rm -f "$ZIP"
ditto -c -k --sequesterRsrc --keepParent "$APP" "$ZIP"

# --------------------------------------------------------------------- dmg ---
if [ "$MAKE_DMG" -eq 1 ]; then
    step "Building $DMG"
    STAGE="$(mktemp -d)"
    ditto "$APP" "$STAGE/$APP_NAME.app"
    ln -s /Applications "$STAGE/Applications"
    rm -f "$DMG"
    hdiutil create -volname "$APP_NAME $VERSION" -srcfolder "$STAGE" \
        -ov -format UDZO -fs HFS+ "$DMG"
    rm -rf "$STAGE"; STAGE=""

    step "Signing and notarizing the disk image"
    codesign --force --timestamp --sign "$IDENTITY" "$DMG"
    notarize "$DMG"
    xcrun stapler staple "$DMG"
fi

# ------------------------------------------------------------------ verify ---
step "Final verification"
spctl -a -vvv -t exec "$APP"
xcrun stapler validate "$APP"
if [ "$MAKE_DMG" -eq 1 ]; then
    xcrun stapler validate "$DMG"
fi

step "Artifacts"
for f in "$ZIP" "$DMG"; do
    if [ -f "$f" ]; then
        printf '    %s  (%s)\n      sha256 %s\n' \
            "$f" "$(du -h "$f" | awk '{print $1}')" "$(shasum -a 256 "$f" | awk '{print $1}')"
    fi
done
printf '\n'
