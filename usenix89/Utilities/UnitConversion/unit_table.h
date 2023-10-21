/* unit_table.h - Copyright 1987 - Gregory R. Simpson */
/* subject to the same conditions outlined in u.c's copyright */

/* ************ a table of the form: ************** 

    struct of_units {
           char  *name;        Name of Unit 
           float value;        Value - Initially set to 0 
           float conversion;   Conversion factor to Primary Unit 
           int   counter;      Number of units in this group 
    } 

The primary unit is the first unit listed in the unit group.
Always include an even number of entries in the group even
if this means that the last entry is blank. Then, set the
number of entries to the correct odd number...this is to
handle displays correctly... For example, see the mass
entry below.

Bug... I should include the Unit Group Menu info in this
file in order to make it easier to include more unit types.
     -simpsong
************************************************** */

unit[30][20] = {
       {
              {"feet",0,1,18},                      /* length */
              {"inches",0,12,18},
              {"yards",0,.33333333,18},
              {"centimeters",0,30.48,18},
              {"meters",0,.3048,18}, 
              {"kilometers",0,3.048e-04,18},
              {"statute miles",0,1.894e-04,18},
              {"nautical miles",0,1.6457925e-04,18},
              {"par-secs",0,9.8827862e-18,18},
              {"light-years",0,3.2218301e-17,18},
              {"mils",0,12000.,18},
              {"microns",0,304800.,18},
              {"millimicrons",0,3.048e08,18},
              {"angstroms",0,3.048e09,18},
              {"x-units",0,3.048e12,18},
              {"rods",0,.060606,18},
              {"fathoms",0,.16666667,18},
              {"furlongs",0,1.51515e-03,18}
       },
       {
              {"pounds",0,1,7},                    /* mass */
              {"grams",0,453.6,7},
              {"kilograms",0,.4536,7},
              {"tons",0,5.0e-04,7},
              {"amus",0,2.732e26,7},
              {"ounces",0,16,7},
              {"stones",0,.0714285,7},
              {"",0,0,7}
       },
       {
              {"m/sec",0,1,6},                     /* speed */
              {"ft/sec",0,3.281,6},
              {"km/hr",0,3.6,6},
              {"cm/sec",0,100,6},
              {"knots",0,1.944,6},
              {"miles/hr",0,2.237,6},
              {"",0,0,7}
       },
       {
              {"cubic meters",0,1,10},                          /* Volume */
              {"cubic cms",0,1e06,10},
              {"liters",0,1000,10},
              {"gallons",0,264.20079,10},
              {"cubic feet",0,35.31,10},
              {"cubic inches",0,6.102e04,10},
              {"barrels",0,11096.433,10},
              {"hogsheads",0,5548.2166,10},
              {"boardfeet",0,8786880.,10},
              {"cords",0,4519.68,10}
       },
       {
              {"square meters",0,1,6},                          /* Area */
              {"square cms",0,1e04,6},
              {"square feet",0,10.76,6},
              {"square inches",0,1550,6},
              {"circular mills",0,1.974e09,6},
              {"acres",0,2.4709661e-04,6}
       },
       {
              {"kg/m3",0,1,5},                          /* Density */
              {"slug/ft3",0,1.940e-03,5},
              {"gm/cm3",0,.001,5},
              {"lb/ft3",0,6.243e-02,5},
              {"lb/in3",0,3.613e-05,5},
              {"",0,0,5}
       },
       {
              {"days",0,1,10},                          /* Time */
              {"years",0,2.738e-03,10},
              {"hours",0,24,10},
              {"minutes",0,1440,10},
              {"seconds",0,8.640e04,10},
              {"decades",0,2.738e-04,10},
              {"score",0,1.369e-04,10},
              {"centuries",0,2.738e-05,10},
              {"millenia",0,2.738e-06,10},
              {"fortnights",0,.0714285,10}
       },
       {
              {"newtons",0,1,5},                          /* Force */
              {"dynes",0,1e05,5},
              {"pounds",0,.2248,5},
              {"gram-force",0,102.0,5},
              {"kilogram-force",0,.1020,5},
              {"",0,0,5}
       },
       {
              {"btus",0,1,9},                          /* Energy */
              {"ergs",0,1.055e10,9},
              {"ft-lbs",0,777.9,9},
              {"hp-hr",0,3.929e-04,9},
              {"joules",0,1055,9},
              {"calories",0,252.0,9},
              {"kilowatt-hours",0,2.930e-04,9},
              {"electron volts",0,6.585e21,9},
              {"MeV",0,6.585e15,9},
              {"",0,0,9}
       },
       {
              {"atmospheres",0,1,12},                  /* Pressure */
              {"dynes/cm2",0,1.013e06,12},
              {"inches of water",0,406.8,12},
              {"cms Hg",0,76.,12},
              {"torr",0,760.,12},
              {"mms Hg",0,760.,12},
              {"inches Hg",0,29.92126,12},
              {"lbs/in2",0,14.70,12},
              {"lbs/ft2",0,2116,12},
              {"newtons/m2",0,1.013e05,12},
              {"bars",0,1.0133,12},
              {"pascals",0,1.013e05,12}
       },
       {
              {"degrees",0,1,5},                          /* Plane Angles */
              {"minutes",0,60,5},
              {"seconds",0,3600,5},
              {"radians",0,1.745e-02,5},
              {"revolutions",0,2.77777778e-03,5},
              {"",0,0,5}
       },
       {
              {"btus/hour",0,1,7},                          /* Power */
              {"ft-lbs/min",0,12.97,7},
              {"ft-lbs/sec",0,.2161,7},
              {"horsepower",0,3.929e-04,7},
              {"kilowatts",0,2.930e-04,7},
              {"watts",0,.2930,7},
              {"calories/sec",0,7.000,7},
              {"",0,0,7}
       },
       {
              {"coulombs",0,1,6},                        /* Electric Charge */
              {"abcoulombs",0,.1,6},
              {"amp-hrs",0,2.778e-04,6},
              {"faradays",0,1.036e-05,6},
              {"statcoulombs",0,2.998e9,6},
              {"electron charges",0,6.2414181e18,6}
       },
       {
              {"gauss",0,1,5},                        /* Magnetic Induction */
              {"kilolines/in2",0,6.452e-03,5},
              {"webers/m2",0,1e-04,5},
              {"tesla",0,1e-04,5},
              {"gamma",0,1e05,5},
              {"",0,0,5}
       },
       {
              {"Footlamberts",0,1,7},                          /* Light */
              {"Nit",0,.2919,7},
              {"Millilamberts",0,.929,7},
              {"Candelas/in2",0,452,7},
              {"Candelas/ft2",0,3.142,7},
              {"Candelas/m2",0,.2919,7},
              {"Stilb",0,2919,7},
              {"",0,1,7}
       },
       {
              {"Btu/(hr-ft2-F/ft)",0,1,4},          /* thermal conductivity */
              {"gm-cal/(sec-cm2-C/cm)",0,.004134,4},
              {"watts/(cm2-C/cm)",0,.01731,4},
              {"kg-cal/(hr-m2-C/m)",0,1.488,4}
       },
       {
              {"Btu/hr-ft2-F",0,1,4},     /* coeff. of heat trans. */
              {"gm-cal/sec-cm2-C",0,.0001355,4},
              {"watts/cm2-C",0,.0005678,4},
              {"kg-cal/hr-m2-C",0,4.882,4}
       },
       {
              {"Btu/hr-ft2",0,1,4},                          /* heat Flux */
              {"gm-cal/sec-cm2",0,.00007535,4},
              {"watts/cm2",0,.0003154,4},
              {"kg-cal/hr-m2",0,2.712,4}
       },
       {
              {"Centipoises",0,1,5},             /* viscosity */
              {"lb/sec-ft",0,.000672,5}, 
              {"lb force-sec/ft2",0,.0000209,5},
              {"lb/hr-ft",0,2.42,5},
              {"kg/hr-m",0,3.60,5},
              {"",0,1,5}
       },
};

