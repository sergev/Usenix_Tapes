# SAMPLE CONNECTIVITY OF THE SUB.DOMAIN.COM DOMAIN
#
#	@(#)sub.domain.com.samp	1.2	10/15/86
#	@(#) Ronald S. Karr <tron@mesa.nsc.com>
#	@(#) National Semiconductor, Sunnyvale
#
#	This file contains the connectivity maps for the top level
#	of the SUB.DOMAIN.COM domain.  All gateways to SUB.DOMAIN.COM
#	must have a current copy of this map.  Also, all other machines
#	within DOMAIN.COM must either have a current copy of this map
#	or be able to forward to a machine that does.

#	Our site
oursite	close-site(DIRECT),
	this-site(DIRECT),
	another-site(LOCAL),
	OUR-ETHERNET

OUR-ETHERNET	= { a,b,c,d }(DEAD)

#    The top level of SUB.DOMAIN.COM
oursite		= oursite.sub.domain.com
close-site	= close-site.sub.domain.com
this-site	= this-site.sub.domain.com

#    Subnets of SUB.DOMAIN.COM
another-site	.sub.sub.domain.com
