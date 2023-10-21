/****************************/
/*      JAMIE'S JUNKER      */
/* "All the News that Fits" */
/*       Version 1.2        */
/*           --             */
/*     check_files and      */
/*   associated functions   */
/****************************/


/*
 * Opens the old article file for reading and a new one for writing;
 * passes file pointers back.  Passes back true iff all ok.
 */
int
setup_files_ok(fname_ca, new_fname_ca, art_file_p, new_art_file_p)
  char fname_ca[];		/* full name of old article file */
  char new_fname_ca[];		/* gets name of new article file */
  FILE **art_file_p;		/* gets file pointer to old */
  FILE **new_art_file_p;	/* gets file pointer to new */
{
  *art_file_p = fopen(fname_ca, "r");
  if (*art_file_p == NULL) {
    printf("Warning: can't open %s\n", fname_ca);
    return 0;
  }
  else {
    sprintf(new_fname_ca, "%s.JUNKER", fname_ca);
    *new_art_file_p = fopen(new_fname_ca, "w");
    if (*new_art_file_p == NULL) {
      fclose(*art_file_p);
      printf("Warning: can't open %s\n", new_fname_ca);
      return 0;
    }
    else
      return 1;
  }
}


/*
 * Cleans up those files opened by setup_files, by unlinking the
 * old and renaming the new to the old.  Returns true iff all ok.
 */
int
cleanup_files(fname_ca, new_fname_ca, art_file, new_art_file)
  char fname_ca[];
  char new_fname_ca[];
  FILE *art_file;
  FILE *new_art_file;
{
  fclose(new_art_file);
  fclose(art_file);

  if (unlink(fname_ca) != 0) {
    printf("Warning: unlink of %s not successful\n", fname_ca);
    return 0;
  }
  else {
    if (rename(new_fname_ca, fname_ca) != 0) {
      printf("Warning: rename of %s not successful\n", new_fname_ca);
      return 0;
    }
    else
      return 1;
  }
}


/*
 * Takes a string, skips over next "word" (sequence of nonblanks)
 * and any bracketing blanks, and returns pointer to start of next word.
 */
char *
next_word(char_ca)
  char char_ca[];
{
  char *temp_pca = char_ca;

  while (*temp_pca == ' ')
    temp_pca++;
  temp_pca = (char *) index(temp_pca, ' ');
  if (temp_pca)
    while (*temp_pca == ' ')
      temp_pca++;

  return temp_pca;
}
  


/*
 * Takes an "Xref: " line from an article and tries to relink each
 * file mentioned in that line to the new version of the file, fname_ca.
 */
relink_xrefs(xref_line_ca, fname_ca)
  char xref_line_ca[],
       fname_ca[];
{
  char xref_fname_ca[MAXNAMLEN+1],
       artname_ca[MAXNAMLEN+1],
      *char_pca;

  debug(TRUE, ("relink_xrefs: xref line:\n  %s\nfname %s\n",
               xref_line_ca,
               fname_ca));

  char_pca = next_word(xref_line_ca); /* at sitename */
  char_pca = next_word(char_pca);     /* at first xref article name */

  while (char_pca && *char_pca) {
    sscanf(char_pca, "%s", artname_ca);
    cnv_to_fname(artname_ca);
    sprintf(xref_fname_ca, "%s/%s", g_newsdir_ca, artname_ca);

    if (strcmp(xref_fname_ca, fname_ca) != 0) {
      /* xref was not to this file */
      debug(TRUE, ("linking %s to %s\n", xref_fname_ca, fname_ca));

      if (unlink(xref_fname_ca) != 0)
        printf("Warning: unlink of %s not successful\n", xref_fname_ca);
      else if (link(fname_ca, xref_fname_ca) != 0)
        printf("Warning: link of %s to %s not successful\n",
               xref_fname_ca,
               fname_ca);
    }

    char_pca = next_word(char_pca);
  }
}


/*
 * Takes an article file and slices out the middle so that the number of
 * bytes in it is less than a maximum.  Passes back a count of number of
 * lines junked; passes back the new length of the file.
 *
 * Algorithm: a new file is opened and the old file is copied over until
 * half the maximum is reached, *and* a blank line (indicating end of
 * header) has been read.  Then lines are thrown away until what's left
 * in the old file, plus the size of the junking message, would make the
 * new file smaller than the maximum.  The tail of the file is copied,
 * the new file is substituted for the old, and all other links to the
 * old file are relinked to the new one.
 */
junk(fname_ca, max_in_file_i, file_length_pi, lines_junked_pi)
  char fname_ca[];
  int max_in_file_i;
  int *file_length_pi,
      *lines_junked_pi;
{
  int message_len_i,
      half_max_i,
      lines_read_i = 0,
      line_length_i,
      left_in_old_i,
      max_left_i;

  FILE *art_file,
       *new_art_file;

  boolean blank_read_b = FALSE,
          xref_read_b = FALSE;

  char prelim_message_ca[MAX_LINE_LEN+1],
       junking_message_ca[MAX_LINE_LEN+1],
       xref_line_ca[MAX_LINE_LEN+1],
       new_fname_ca[MAXNAMLEN+1],
       line_ca[MAX_LINE_LEN+1];

  debug(TRUE, ("junking %s\n", fname_ca));

  if (setup_files_ok(fname_ca, new_fname_ca, &art_file, &new_art_file)) {
    sprintf(prelim_message_ca,
            "\n\\ /\n X [%s: junked %%d lines]\nO O\n\n",
            g_sitename_ca);
    message_len_i = strlen(prelim_message_ca) + 1;

    half_max_i = max_in_file_i / 2;
    left_in_old_i = *file_length_pi;
    *file_length_pi = 0;

    debug(TRUE, ("copying"));
    /*
     * copy lines until (a) half limit copied, and (b) either (b1) the end
     * of the header has been reached, or else (b2) MAX_HEADER_LINES worth
     * of header have been processed (indicating that the poster has rather
     * ingeniously tried to turn his whole article into a header)
     */
    while (  ! (  *file_length_pi >= half_max_i
               && (  blank_read_b
                  || lines_read_i > MAX_HEADER_LINES))
          && (fgets(line_ca, sizeof(line_ca), art_file) != NULL)) {
      debug(TRUE, ("."));
      lines_read_i++;

      line_length_i = strlen(line_ca);
      if (line_length_i <= 1)
        blank_read_b = TRUE;
      left_in_old_i -= line_length_i;

      fputs(line_ca, new_art_file);
      *file_length_pi += line_length_i;

      if ((!xref_read_b) && strncmp(line_ca, "Xref:", 5) == 0) {
        strcpy(xref_line_ca, line_ca);
        xref_read_b = TRUE;
      }
    }

    max_left_i = max_in_file_i - (*file_length_pi + message_len_i);

    debug(TRUE, ("\nlast line copied:\n%sjunking", line_ca));
    /* eat lines until half limit left */
    *lines_junked_pi = 0;
    while (  left_in_old_i > max_left_i
          && (fgets(line_ca, sizeof(line_ca), art_file) != NULL)) {
      debug(TRUE, ("."));
      *lines_junked_pi += 1;
      left_in_old_i -= strlen(line_ca);
    }

    sprintf(junking_message_ca, prelim_message_ca, *lines_junked_pi);
    fputs(junking_message_ca, new_art_file);
    *file_length_pi += strlen(junking_message_ca);

    debug(TRUE, ("\ncopying"));
    /* copy tail of file */
    while (fgets(line_ca, sizeof(line_ca), art_file) != NULL) {
      debug(TRUE, ("."));
      fputs(line_ca, new_art_file);
      *file_length_pi += strlen(line_ca);
    }

    debug(TRUE, ("\n"));
    if (  cleanup_files(fname_ca, new_fname_ca, art_file, new_art_file)
       && xref_read_b)
      relink_xrefs(xref_line_ca, fname_ca);
  }
}

  
/*
 * Checks all the files in the directory array past the first one vulnerable
 * and makes sure their total size is less than limit_i.
 *
 * Algorithm: goes through files sequentially.  At each file, the maximum
 * size for the particular file is approximated by dividing the number of
 * bytes left to the total limit by the number of files left in the directory
 * entry array.  If the size is higher than the limit, the file is junked.
 * In any case, the new size is deducted from the amount left to the limit.
 *
 * A "fair" algorithm, which would take some extra time, would be to resort
 * the directory entries by increasing size of file before doing the scan
 * of all the files.  This would cut down only the biggest files, and cut
 * them all down to approx. the same size.  As it is now, a big file is
 * junked more if it appears earlier in the list.
 */
check_files(dirname_ca, direct_pa, first_vuln_i, no_files_i, limit_i)
  char dirname_ca[];		/* name of news directory */
  direct_p_t direct_pa[];	/* directory entry array (scandir */
  int first_vuln_i,		/* first file vulnerable (first_vuln) */
      no_files_i,		/* no of files in directory array */
      limit_i;			/* total limit of size of all vuln files */
{
  int i,
      left_to_limit_i,
      max_in_file_i,
      file_length_i,
      files_junked_i = 0,
      lines_junked_i,
      total_lines_junked_i = 0,
      orig_bytes_i = 0,
      new_bytes_i = 0;

  char fname_ca[MAXNAMLEN+1];

  struct stat file_stat;

  debug(TRUE, ("check_files on %s\n", dirname_ca));
  
  left_to_limit_i = limit_i;
  for (i=first_vuln_i; i<no_files_i; i++) {
    max_in_file_i = left_to_limit_i / ((no_files_i-i)+1);

    sprintf(fname_ca, "%s/%s", dirname_ca, direct_pa[i]->d_name);
    stat(fname_ca, &file_stat);
    file_length_i = file_stat.st_size;
    orig_bytes_i += file_length_i;

    debug(TRUE, ("%s: size %d, max %d\n",
                 fname_ca,
                 file_length_i,
                 max_in_file_i));

    if (file_length_i > max_in_file_i) {
      junk(fname_ca, max_in_file_i, &file_length_i, &lines_junked_i);
      files_junked_i++;
      total_lines_junked_i += lines_junked_i;
    }
    else
      debug(TRUE, ("preserving \n"));

    new_bytes_i += file_length_i;
    left_to_limit_i -= file_length_i;
  }

  debug(TRUE, ("check_files: about to return\n\n"));
  printf("  %d files vulnerable, %d affected; junked %d lines, %d bytes (%d%%)\n\n",
         no_files_i - first_vuln_i,
         files_junked_i,
         total_lines_junked_i,
         orig_bytes_i - new_bytes_i,
         (100 * (orig_bytes_i - new_bytes_i)) / orig_bytes_i);
}

