#include "inclusions.c"


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: errhand                                                    */
/*                                                                          */
/*                                                                          */
/*     PURPOSE: REPORT THE ERROR AND WHAT FUNCTION IT TOOK PLACE IN TO THE  */
/*              USER VIA THE TERMINAL CURRENTLY BEING USED.                 */
/*                                                                          */
/****************************************************************************/
errhand(funcname,errnum)
   char *funcname;
   int errnum;
   {
   static char *errtable[52];

   errtable[0] = "The CORE SYSTEM has already been initialized.";
   errtable[1] = "The specified level cannot be supported.";
   errtable[2] = "The surface has already been initialized.";
   errtable[3] = "No physical surface is associated with the specified logical surface.";
   errtable[4] = "The CORE SYSTEM has not been initialized.";
   errtable[5] = "The specified surface has not been initialized.";
   errtable[6] = "The specified surface is already selected.";
   errtable[7] = "The specified surface was not selected.";
   errtable[8] = "A segment is open.";
   errtable[9] = "The specified surface is not selected.";
   errtable[10] = "The specified surface has not been deselected.";
   errtable[11] = "This function has already been called once.";
   errtable[12] = "A segment has been opened.";
   errtable[13] = "A value specified for a default attribute is improper.";
   errtable[14] = "The specified segment does not exist.";
   errtable[15] = "The VIEW SURFACE ARRAY is not large enough.";
   errtable[16] = "The segment name list is not large enough.";
   errtable[17] = "There has been no 'enbatch' since last 'bebatch'.";
   errtable[18] = "There has been no corresponding 'bebatch'.";
   errtable[19] = "A viewing function has been invoked,or a segment has been created.";
   errtable[20] = "The value for TYPE is improper.";
   errtable[21] = "No segment is open.";
   errtable[22] = "n is <= 0.";
   errtable[23] = "String contains an illegal character.";
   errtable[24] = "The vectors established by the CHARSPACE and CHARPLANE attribute specifications are parallel.";
   errtable[25] = "Invalid marker table offset.";
   errtable[26] = "Invocation when no open segment.";
   errtable[27] = "Invalid attribute value.";
   errtable[28] = "Invalid segment type.";
   errtable[29] = "Invalid (i.e., non-existing) segment.";
   errtable[30] = "Invalid image transformation for the segment.";
   errtable[31] = "A retained segment named SEGNAME already exists.";
   errtable[32] = "The type of retained segment to be created is inconsistent with the current attribute value for the IMAGE_TRANSFORMATION attribute.";
   errtable[33] = "No view surface is currently selected.";
   errtable[34] = "The current viewing specification is inconsistent.";
   errtable[35] = "No view surfaces have been initialized.";
   errtable[36] = "There is an existing retained segment named NEW_NAME.";
   errtable[37] = "There is no retained segment named SEGMENT_NAME.";
   errtable[39] = "Delta_x,delta_y,delta_z, are all zero: no view up direction can be established.";
   errtable[42] = "'ndcsp2' or 'ndcsp3' has already been invoked since the CORE SYSTEM was  last initialized.";
   errtable[43] = "The invocation of 'ndcsp2' ('ndcsp3') is too late - the default values   for normalized device coordinates have already been assumed, and hence cannot be changed.";
   errtable[44] = "A parameter value is greater than 1,or is less than or equal to 0.";
   errtable[45] = "Neither parameter has a value of 1.";
   errtable[46] = "Viewport extent is outside of normalized device coordinate space.";
   errtable[47] = "XMIN is not less than XMAX, or YMIN is not less than YMAX.";
   errtable[48] = "Specified device already enabled.";
   errtable[49] = "DEVICE_CLASS or DEVICE_NUM invalid.";
   errtable[50] = "DEVICE_CLASS invalid.";
   errtable[51] = "Specified device is not enabled.";
/****************************************************************************/
/* Version 7:                                                               */
/* static char *errtable[] = {                                              */
/*    "The CORE SYSTEM has already been initialized.",                      */
/*    "The specified level cannot be supported.",                           */
/*    "The surface has already been initialized.",                          */
/*    "No physical surface is associated with the specified logical surface.",
      "The CORE SYSTEM has not been initialized.",                          */
/*    "The specified surface has not been initialized.",                    */
/*    "The specified surface is already selected.",                         */
/*    "A segment is open.",                                                 */
/*    "The specified surface is not selected.",                             */
/*    "The specified surface has not been deselected.",                     */
/*    "This function has already been called once.",                        */
/*    "A segment has been opened.",                                         */
/*    "A value specified for a default attribute is improper.",             */
/*    "The specified segment does not exist.",                              */
/*    "The VIEW SURFACE ARRAY is not large enough.",                        */
/*    "The segment name list is not large enough.",                         */
/*    "There has been no 'enbatch' since last 'bebatch'.",                  */
/*    "There has been no corresponding 'bebatch'.",                         */
/*    "A viewing function has been invoked,or a segment has been created.", */
/*    "The value for TYPE is improper.",                                    */
/*    "No segment is open.",                                                */
/*    "n is <= 0.",                                                         */
/*    "String contains an illegal character.",                              */
/*    "The vectors established by the CHARSPACE and CHARPLANE attribute specif
ications are parallel.",                                                    */
/*    "Invalid marker table offset.",                                       */
/*    "Invocation when no open segment.",                                   */
/*    "Invalid attribute value.",                                           */
/*    "Ivalid segment type.",                                               */
/*    "Invalid (i.e., non-existing) segment.",                              */
/*    "Invalid image transformation for the segment.",                      */
/*    "A retained segment named SEGNAME already exists.",                   */
/*    "The type of retained segment to be created is inconsistent with the cur
rent attribute value for the IMAGE_TRANSFORMATION attribute.",              */
/*    "No view surface is currently selected.",                             */
/*    "The current viewing specification is inconsistent.",                 */
/*    "No view surfaces have been initialized.",                            */
/*    "There is an existing retained segment named NEW_NAME.",              */
/*    "There is no retained segment named SEGMENT_NAME.",                   */
/*    "No characters in string (n=0).",                                     */
/*    "Delta_x,delta_y,delta_z, are all zero: no view up direction can be esta
blished.",                                                                  */
/*    "UMIN is not less than UMAX, or VMIN is not less than VMAX.",         */
/*    "FRONT_DISTANCE is greater than BACK_DISTANCE, so that the back clipping
plane is in front of the front clipping plane.",                            */
/*    "'ndcsp2' or 'ndcsp3' has already been invoked since the CORE SYSTEM was
 last initialized.",                                                        */
/*    "The invocation of 'ndcsp2' ('ndcsp3') is too late - the default values
for normalized device coordinates have already been assumed, and hence cannot
be changed.",                                                               */
/*    "A parameter value is greater than 1, or is less than or equal to 0.",*/
/*    "Neither parameter has a value of 1.",                                */
/*    "Viewport extent is outside of normalized device coordinate space.",  */
/*    "XMIN is not less than XMAX, or YMIN is not less than YMAX.",         */
/*    "Specified device already enabled.",                                  */
/*    "DEVICE_CLASS or DEVICE_NUM invalid.",                                */
/*    "DEVICE_CLASS invalid.",                                              */
/*    "Specified device is not enabled.",                                   */
/*    "LOCATOR_NUM is invalid.",                                            */
/*    "The specified LOCATOR device is not enabled.",                       */
/*    "VALUATOR_NUM is invalid.",                                           */
/*    "The specified VALUATOR device is not enabled.",                      */
/*    "The TIME value is less than zero.",                                  */
/*    "EVENT_CLASS and EVENT_NUM do not specify a valid event device.",     */
/*    "EVENT_CLASS is not a legal event device class.",                     */
/*    "The specified association already exists.",                          */
/*    "EVENT_CLASS or SAMPLED_CLASS reference invalid classes or a class of th
e wrong type.",                                                             */
/*    "EVENT_NUM or SAMPLED_NUM are invalid device numbers for their respectiv
e classes.",                                                                */
/*    "The specified association does not exists.",                         */
/*    "The current event report is not from a PICK device.",                */
/*    "The current event report is not from a KEYBOARD event.",             */
/*    "Input string was not large enough to hold the string centered by the op
erator.",                                                                   */
/*    "When the event occurred, the specified LOCATOR device was not enabled o
r was not associated with the event device that caused the current event repor
t.",                                                                        */
/*    "When the event occurred, the specified VALUATOR device was not enabled
or was not associated with the event device that caused the current event repo
rt.",                                                                       */
/*    "XECHO and YECHO specify positions outside normalized device coordinate
space.",                                                                    */
/*    "DEVICE_CLASS and DEVICE_NUM specify a non-existent device.",         */
/*    "LOCATOR_NUM does not specify a valid LOCATOR device.",               */
/*    "XLOC,YLOC specify a position outside normalized device coordinate space
.",                                                                         */
/*    "VALUATOR_NUM is not a valid VALUATOR device.",                       */
/*    "LOW_VALUE is greater than HIGH_VLAUE.",                              */
/*    "INITIAL_VALUE does not lie in the range defined by LOW_VALUE and HIGH_V
ALUE.",                                                                     */
/*    "KEYBOARD_NUM is not a valid KEYBOARD device.",                       */
/*    "BUFFER_SIZE is less than zero or greater than an implementation defined
maximum.",                                                                  */
/*    "BUTTON_NUM is not a valid BUTTON device.",                           */
/*    "Incorrect arguments for the specified function.",                    */
/*    "Incorrect argument count for the specified function.",               */
/*    "Specified function not supported."};                                 */
/*                                                                          */
/****************************************************************************/

   printf("%s:%s\n",funcname,errtable[errnum]);
/********************************************************************/
/* Version 7: fprintf(stderr,"%s:%s\n",funcname,errtable[errnum]);  */
/*            with corresponding #include <stdio.h> at top of file. */
/********************************************************************/

   /*******************************************************************/
   /***                                                             ***/
   /*** INCREMENT THE ERROR NUMBER BEFORE PLACING IN COMMON BECAUSE ***/
   /*** 0 IS NOT A LEGAL ERROR NUMBER IN THE ROUTINE 'repterr'      ***/
   /*** WHICH USES THE VARIABLE SOLELY. RANGE FOR 'erreport' IS     ***/
   /*** FORCED TO BE FROM 1 TO N+1 INSTEAD OF FROM O TO N.          ***/
   /***                                                             ***/
   /*******************************************************************/

   erreport = ++errnum;
   return;
   }


/****************************************************************************/
/*                                                                          */
/*     FUNCTION: reportmostrecenterror                                      */
/*                                                                          */
/*     PURPOSE: COPIES THE ERROR REPORT FOR THE MOST RECENTLY DETECTED      */
/*              ERROR INTO 'error'.                                         */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*     NOTE: THE ERROR NUMBER RETURNED TO USER WILL BE ONE GREATER THAN     */
/*           INDEX INTO ERROR HANDLER TABLE BECAUSE                         */
/*              a) C ARRAY INDICES RUN FROM 0 TO N                          */
/*              b) ROUTNE RETURNS A 0 IF NO ERROR HAS OCCURED SINCE LAST    */
/*                 CALL TO THIS ROUTINE, BUT 0 HAPPENS TO BE A LEGAL INDEX  */
/*                 INTO ERROR HANDLER TABLE.                                */
/*           THUS, ERROR NUMBER WILL RANGE FROM 1 TO N+1 CORRESPONDING TO   */
/*           0 TO N IN ERROR HANDLER ROUTINE.                               */
/*                                                                          */
/****************************************************************************/

reportmostrecenterror(error)
   int *error;
   {

   *error = erreport;
   erreport = 0;

   return(0);
   }


