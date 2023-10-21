#include "hd.h"
#include "mydir.h"

dircmd (cmd) register cmd; 
{
  static int eotflag;
  register ret;	/* return value */
  register int i;
  int scmd, selflag;
  int putch();
  char oldvimotion;
  extern int didlong;		/* from longlist() */

  ret = REPLOT;
  scmd = cmd;
  selflag = (VSHMODE == SELECTMODE);
  /* Defaults to EOT */
  if (cmd == (QUITCHAR-'@')) {
	if (eotflag++) {
		clearmsg(2);
		printf("\r\n\n");
		leave ();
	}
	putmsg("(Quit)");
	return NOREPLOT;
  }
  else {
    eotflag = 0;
    if ((cmd == '\'' || (cmd == '\r' && cmd != PAGECHAR-'@')) && selflag) {
	if (*selectname)
		cmd = 'a'+selectcmd;
	else {
		putmsg("Nothing selected");
		return NOREPLOT;
	}
    }
    else if (VSHMODE == SELECTMODE && VIMOTION)
	cmd = vi_motion(cmd);
    if (cmd == LF)
	ret = enterdir (DOTDOT);
    else if (tfiles == 0) {
	putmsg("No files");
	return NOOP;
    }
    else if ((cmd == '+') || (column < 2 && cmd == CTLF)) {
	cpage += pageoff ? pageoff : 1;
	if (cpage > tpages)
		cpage = 1;
	pageoff = 0;
    }
    else if ((cmd == EOT) || (cmd == ESNP) || (cmd == (PAGECHAR-'@'))) {
	if (cpage == tpages)
		cpage = 1;
	else {
		cpage += column > 1 ? column-1 : 1;
		if (cpage > tpages)
			cpage = tpages;
	}
	pageoff = 0;
    }
    else if (cmd == TABCMD || cmd == ESRT || cmd == ESLF) {
	if (column > 1) {
		i = pageoff;
		/* Cleanup display after longlisting */
		if (didlong)
			dispdir(0);
		if (cmd == ESLF) {
			if (pageoff == 0) {
				pageoff = column - 1;
				if ((cpage+pageoff) >= tpages)
					pageoff = tpages - cpage;
			}
			else
				pageoff--;
		}
		else {
			if ((cpage+pageoff) >= tpages || pageoff >= (column-1))
				pageoff = 0;
			else 
				pageoff++;
		}
		bufout();
		atxy(OFFFILE+colfield*i, 2);
		printf(" ");
		colprompt();
		/* Fix selected item */
		if (cpage != tpages && *selectname) {
			atfile(selectcmd, OFFARROW+colfield*i);
			printf("  ");
			*selectname = 0;
			/* Kludge ! */
			oldvimotion = VIMOTION;
			VIMOTION = 0;
			dircmd(selectcmd+'a');
			VIMOTION = oldvimotion;
		}
		unbufout();
	}
	ret = NOREPLOT;
    }
    else if ((cmd >= 'a' && cmd <= 'z') || cmd == ESUP || cmd == ESDN)
    {
	i = pgend()-1;
	if (cmd == ESUP) {
		if (*selectname) {
			if (i == 0)
				return NOREPLOT;	
			cmd = selectcmd-1;
			if (cmd < 0)
				cmd = i;
		}
		else
			cmd = selflag ? 0 : i+1;
	}
	else if (cmd == ESDN) {
		if (*selectname) {
			if (i == 0)
				return NOREPLOT;
			cmd = selectcmd+1;
			if (cmd > i)
				cmd = 0;
		}
		else
			cmd = selflag ? 0 : i+1;
	}
	else
		cmd -= 'a';
	if (cmd > i)
		ret = NOOP;
	else if (selflag && (*selectname == 0 || selectcmd != cmd)) {
		if (*selectname) {
			atfile(selectcmd, OFFARROW+colfield*pageoff);
			printf("  ");
		}
		selectcmd = cmd;
		selectpage = cpage+pageoff;
		atfile(cmd, OFFARROW+colfield*pageoff);
		printf(arrow);
		strcpy(selectname, filename(cmd));
		ret = NOREPLOT;
	}
	else if (enterfile (filename (cmd)) == NOREPLOT) 
	{
/*
	    atfile(cmd, OFFARROW+colfield*pageoff); printf("  ");
	    longfile (cmd);
 */
	    ret = NOREPLOT;
	}
    }
    else if (cmd == ESBS || cmd == ESFS) {
	/* i is max of window */
	ret = (cmd == ESFS);
	if (*selectname)
		i = *selectname;
	else {
		i = ret ? pgend()-1 : 0;
		i = (filename(i))[0];
	}
	if (ret)
		i++;
	else
		i--;
	ret = findc(i, ret);
    }
    else if (cmd > '0' && cmd <= '9') {
	cpage = cmd - '0';
	pageoff = 0;
    }
    else if (cmd == '0') {
	cpage = tpages;
	pageoff = 0;
    }
    else if (cmd == '-')
    {
	cpage--;
	if (cpage < 1) cpage = tpages;
	pageoff = 0;
    }
    else if (cmd == CTLU) {
	if (cpage == 1)
		cpage = tpages;
	else {
		cpage -= column > 1 ? column-1 : 1;
		if (cpage < 1)
			cpage = 1;
	}
	pageoff = 0;
    } 
    else if (cmd == CTLL)
	*selectname = 0;
    else
	ret = NOOP;
  }
  /* VI style motion commands */
  if (scmd != cmd && ret == NOOP && 'h' <= scmd && scmd <= 'l' && scmd != 'i') {
	ret = dircmd(vi_motion(scmd));
  }
  if (cpage+pageoff != selectpage)
	*selectname = 0;
  return ret;
}

colprompt()
{
	atxy(OFFFILE+colfield*pageoff, 2);
	tputs(BO, 0, putch);
	printf("V");
	tputs(BE, 0, putch);
}

vi_motion(cmd)
register int cmd;
{
	switch(cmd) {
	case 'h':
		cmd = ESLF;
		break;
	case 'j':
		cmd = ESDN;
		break;
	case 'k':
		cmd = ESUP;
		break;
	case 'l':
		cmd = ESRT;
		break;
	}
	return cmd;
}
