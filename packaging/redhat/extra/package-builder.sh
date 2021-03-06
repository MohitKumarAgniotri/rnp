#!/bin/bash
#
# (c) 2017 Ribose Inc.
# Frank Trampe, Jeffrey Lau and Ronald Tse.
#
# It is hereby released under the license of the enclosing project.
#
# Preconditions:
#  - $ ls
#    rnp/
#
# Call this with:
#
#  - the desired version number as the first argument;
#  - the desired rpm release number as the second argument;
#
#  - the path where the source code is as the third optional argument;
#    (default: the same as the package name, $PNAME.
#
#  - the directory to place the generated spec file as the fourth optional
#    argument;
#    (default: ~/rpmbuild/SPECS)
#
#  - env var $PACKAGER, the PACKAGER identity
#
#  - env var $SIGN (default: true), set an empty string to make it not sign the
#    RPM

readonly __progname="$(basename $0)"

usage() {
  echo "Usage: ${__progname} <version> [source_path] [target_spec_dir]" >&2
}

main() {
  # Make sure at least the version is supplied.
  if [ $# -lt 1 ]; then
    usage
    exit 1
  fi

  readonly local PNAME=rnp
  readonly local PVERSION="$1"
  readonly local PRELEASE="$2"
  readonly local PPATH="${3:-${PNAME}}"
  readonly local SPEC_DIR="${4:-${HOME}/rpmbuild/SPECS}"
  readonly local PNAMEVERSION="${PNAME}-${PVERSION}"
  readonly local SOURCES_DIR="${SOURCES_DIR:-${HOME}/rpmbuild/SOURCES}"

  echo "Create the rpm spec file"
  sh packaging/redhat/extra/spec-builder.sh "$@"

  echo "Create the SOURCES_DIR ${SOURCES_DIR}"
  mkdir -p "${SOURCES_DIR}"

  # Make the source tarball for the build.
  git archive --format=tar --prefix=${PNAMEVERSION}/ HEAD | bzip2 > ${PNAMEVERSION}.tar.bz2

  # Copy the source tarball into the source directory for rpmbuild.
  readonly local PSOURCE_PATH="${SOURCES_DIR}/${PNAMEVERSION}.tar.bz2"
  cp -pRP "${PNAMEVERSION}.tar.bz2" "${PSOURCE_PATH}"
  chown $(id -u):$(id -g) "${PSOURCE_PATH}"

  readonly local PSPEC_PATH="${SPEC_DIR}/${PNAME}.spec"

  # Build the packages.
  if [[ -z "${SIGN}" ]]; then
    local sign_opts=
  else
    local sign_opts=--sign
  fi
  echo rpmbuild -v -ba ${sign_opts} --nodeps "${PSPEC_PATH}"
  rpmbuild -v -ba ${sign_opts} --nodeps "${PSPEC_PATH}"
}

main "$@"

# vim:sw=2:et
