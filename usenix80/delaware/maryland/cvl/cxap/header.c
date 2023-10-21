#
#include        "/mnt/phil/cxap/area.define"

int header(file,buffer)
char    *file;                          /*input file                    */
int     *buffer;                        /*header buffer                 */
{
	register
	int     sourcf,                 /*source file                   */
		status;                 /*status of system call         */

	if ((sourcf = open(file,READ)) == ERROR)
	{
		printf(2,"Source file not opened\n");
		return(ERROR);
	}
					/*read picture header           */
	status = read(sourcf,buffer,HLEN);
	if ((status == ERROR) || (status == EOF))
	{
		printf(2,"Source header not read\n");
		return(status);
	}
	if (close(sourcf) == ERROR)
	{
		printf(2,"Source file not closed\n");
		return(ERROR);
	}

	return(SUCCES);

} /*end header*/
