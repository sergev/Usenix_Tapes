/*
 * star.c - basic test for TIG-pack
 */

main()
	{
	penup();
	movepen( 0 , 0 );
	pendown();
	movepen( 6000 , 5000 );

	penup();
	movepen( 0 , 2500 );
	pendown();
	movepen( 6000 , 2500 );

	penup();
	movepen( 0 , 5000 );
	pendown();
	movepen( 6000 , 0 );

	penup();
	movepen( 3000 , 5000 );
	pendown();
	movepen( 3000 , 0 );
	}
