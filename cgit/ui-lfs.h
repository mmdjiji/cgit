#ifndef UI_LFS_H
#define UI_LFS_H

void cgit_lfs_handle(void);

/*
 * Parse an LFS pointer from a git blob buffer.
 * Returns 1 if it's a valid LFS pointer, 0 otherwise.
 * On success, oid_out is filled with the 64-char hex OID (null-terminated)
 * and *size_out is filled with the declared file size.
 */
int cgit_lfs_parse_pointer(const char *buf, unsigned long size,
			   char *oid_out, unsigned long *size_out);

/*
 * Build the filesystem path for an LFS object and check if it exists.
 * Returns a malloc'd path string, or NULL if the object doesn't exist.
 * Caller must free the returned string.
 */
char *cgit_lfs_object_path(const char *oid);

#endif /* UI_LFS_H */
