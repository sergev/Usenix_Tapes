/*
 * Structure used by pwparse(III).
 * Parsed version of /etc/passwd entry.
*/

struct passwd
	{
	char *user;
	char *pw;
	int uid;
	int gid;
	char *gcos;
	char *homedir;
	char *shell;
	};
