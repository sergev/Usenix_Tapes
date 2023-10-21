/****************************/
/*      JAMIE'S JUNKER      */
/* "All the News that Fits" */
/*       Version 1.2        */
/*           --             */
/*      first_vuln and      */
/*   associated functions   */
/****************************/


/*
 * Takes the number of hours in the window; finds out what time it was
 * that many hours before the present, and puts that in stime_ia in
 * time array format.
 */
get_start_time(stime_ia, window_hrs_i)
  int stime_ia[];
  int window_hrs_i;
{
  long stime_l;

  char *start_pca,
       day_ca[4],
       smon_ca[4];

  stime_l = time(0) - (long)(window_hrs_i*60*60);
  start_pca = ctime(&stime_l);
  debug(TRUE, ("start time %s", start_pca));
  sscanf(start_pca, " %s %s %d %d:%d:%d 19%d",
         day_ca, smon_ca, &stime_ia[2], 
         &stime_ia[3], &stime_ia[4], &stime_ia[5],
         &stime_ia[0]);
  stime_ia[1] = month_no(smon_ca);
}


/*
 * Scans file artname_ca in dirname_ca in order to find out what date
 * it was received at.  Cannot use create date for this, as file could
 * have been changed by a previous junker run.
 * Many tedious error conditions.
 */
get_recd_time(dirname_ca, artname_ca, time_ia)
  char dirname_ca[];
  char artname_ca[];
  int time_ia[];
{
  char fname_ca[MAXNAMLEN+1],
       day_ca[4],
       mon_ca[4],
       line_ca[MAX_LINE_LEN+1],
      *test_pca;

  FILE *art_file;

  sprintf(fname_ca, "%s/%s", dirname_ca, artname_ca);

  time_ia[0] = time_ia[1] = time_ia[2] =
  time_ia[3] = time_ia[4] = time_ia[5] = 0; /* if something goes wrong */

  art_file = fopen(fname_ca, "r");
  if (art_file == NULL)
    printf("Warning: can't open %s\n", fname_ca);
  else {
    do
      test_pca = fgets(line_ca, sizeof(line_ca), art_file);
    while (  test_pca != NULL
          && strncmp(line_ca, "Path:", 5) != 0);

    if (test_pca == NULL)
      printf("Warning: %s: no Path line", fname_ca);
    else {
      sscanf(line_ca, "Path: %[^!]", g_sitename_ca);

      do
        test_pca = fgets(line_ca, sizeof(line_ca), art_file);
      while (  test_pca != NULL
            && strncmp(line_ca, "Date-Received:", 14) != 0);

      if (test_pca == NULL)
        printf("Warning: %s: no Date-Received line", fname_ca);
      else {
        debug(TRUE, ("%s:%s", fname_ca, line_ca));
        if (sscanf(line_ca, "Date-Received: %s %d-%3c-%d %d:%d:%d", day_ca,
                   &time_ia[2], mon_ca, &time_ia[0],      /* e.g. 28-Apr-86 */
                   &time_ia[3], &time_ia[4], &time_ia[5]) /* e.g. 17:57:01  */
            == 7)
          time_ia[1] = month_no(mon_ca);
        else
          printf("Warning: %s: Bad Date-Received line", fname_ca);
      }
    }

    fclose(art_file);
  }
}


/*
 * Returns the directory entry array index of the first article file in
 * the directory which is vulnerable to junking.  Does this by binary
 * search on the directory entries => takes time log(n), where n is the
 * number of entries in the directory.
 */
int
first_vuln(dirname_ca, direct_pa, no_files_i, window_hrs_i)
  char dirname_ca[];		/* full name of directory */
  direct_p_t direct_pa[];	/* pointer to direct pointers (scandir) */
  int no_files_i,		/* no of entries in array */
      window_hrs_i;		/* window, in hours (input) */
{
  int j,
      high_i, low_i, mid_i,
      stime_ia[6],
      time_ia[6];

  debug(TRUE, ("first_vuln on %s, %d files", dirname_ca, no_files_i));
  get_start_time(stime_ia, window_hrs_i);

  high_i = no_files_i-1;
  low_i = 0;
  while (high_i >= low_i) { /* binary search */
    mid_i = (high_i + low_i) / 2;
    debug(TRUE, ("first_vuln dir.entry %d, art. %s...\n",
                 mid_i,
                 direct_pa[mid_i]->d_name));

    get_recd_time(dirname_ca,
                  direct_pa[mid_i]->d_name,
                  time_ia);

    for (j = 0; j <= 5; j++) { /* compare parts of date */
      if (time_ia[j] < stime_ia[j]) {
        debug(TRUE, ("  low\n"));
        low_i = mid_i+1;
        break;
      }
      if (time_ia[j] > stime_ia[j]) {
        debug(TRUE, ("  high\n"));
        high_i = mid_i-1;
        break;
      }
    }

    if (j > 5) { /* time is exactly equal, treat as high */
      debug(TRUE, ("  equal\n"));
      high_i = mid_i-1;
    }
  } /* end binary search */

  /* low_i points to first article received after */
  /* start date - work it out for yourself        */
  debug(TRUE, ("first vuln is dir.entry %d\n", low_i));
  return low_i;
}

