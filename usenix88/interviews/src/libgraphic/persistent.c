/*
 * Persistent class implementation.  Persistent is the general persistent
 * object from which Graphic is derived.
 */

#include <stdio.h>
#include <InterViews/Graphic/file.h>
#include <InterViews/Graphic/objman.h>
#include <InterViews/Graphic/persistent.h>

Persistent::Persistent () {} 

void Persistent::Warning (const char* msg) {
    fflush(stdout);
    fprintf(stderr, "warning: %s\n", msg);
}

void Persistent::Panic (const char* msg) {
    fflush(stdout);
    fprintf(stderr, "internal error: %s\n", msg);
    exit(2);
}

void Persistent::Fatal (const char* msg) {
    fflush(stdout);
    fprintf(stderr, "%s\n", msg);
    exit(2);
}

UID Persistent::getUID () {
    return TheManager->GetUID( this );
}

boolean Persistent::write (File*) {
    if (TheManager->Update(this)) {
	Clean();
	return true;
    } else {
	return false;
    }
}

Persistent::~Persistent () {
    if (
	TheManager != nil && this != TheManager &&
	!TheManager->Invalidate(this)
    ) {
	Panic("couldn't invalidate object during destruction");
    }
}

boolean Persistent::Save () {
    return IsDirty() ? TheManager->Store(this) : true;
}

boolean Persistent::writeObjects (File* f) {
    return write(f);
}

boolean Persistent::IsDirty () {
    if (TheManager == nil) {
	return true;
    } else {
	return TheManager->ObjectIsDirty(this);
    }
}

void Persistent::Touch () {
    if (TheManager != nil) {
	TheManager->ObjectTouch(this);
    }
}

void Persistent::Clean () {
    if (TheManager != nil) {
	TheManager->ObjectClean(this);
    }
}
