/* Round 24 diagnostic: the last writers of the stack slots 0x1000aa9e reads unset (lastwriter.c). */
#ifndef HG_LAUNCHER_LASTWRITER_H
#define HG_LAUNCHER_LASTWRITER_H

/* True in a watcher run (hgrc.cfg "uninittrace", or "lastwriter" in hgrc.cfg / hgrc_extra.cfg) of the parent, not in its child. */
int LastWriter_Wanted(void);

/* Runs the sim with cmd in a child under the debugger; its exit code to *result. 0 when it could not start. */
int LastWriter_RunSim(const char *cmd, int *result);

#endif
