#!/bin/sh
PORT_DIR=go
fetch() {
    run_fetch_web "https://storage.googleapis.com/golang/go1.4.3.linux-amd64.tar.gz"
    run_patch serenity-changes.patch -p1
}
configure() {
    echo ""
}
build() {
    # FIXME: the script crashes on host because it's compiled for serenity.
    (cd go/src && CGO_ENABLED=0 ./make.bash) || true
}
install() {
    # We need the go boostrap directory in order to use the bootstrap tool
    # FIXME: gcc does not exist inside serenity :(
    GO_BOOTSTRAP_DIR="$SERENITY_ROOT/Base/home/anon/go" |
    [ -d "$GO_BOOTSTRAP_DIR" ] && rm -r "$GO_BOOTSTRAP_DIR"
    cp -r go "$SERENITY_ROOT/Base/home/anon/"
}
. ../.port_include.sh
