# verssion control hash

exists($$PWD/.git) {
    REVISION = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --long)
}

isEmpty( REVISION ) {
    message("This is not git version!")
    REVISION = "1.0.0"
}

DEFINES += GITHASH=\\\"$$REVISION\\\"
