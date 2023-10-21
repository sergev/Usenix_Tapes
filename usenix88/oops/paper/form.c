#include "Form.h"
#include "NumberField.h"
#include "AlphaField.h"

main()
{
	Form& aForm = *new Form;
	aForm.addField(
		*new NumberField(
			"Part Number",
			Rectangle(Point(10,20),Point(50,30)),
			123654));
	aForm.addField(
		*new AlphaField(
			"Description",
			Rectangle(Point(60,20),Point(100,30)),
			"large blue widget"));
	aForm.storeOn(cout);
}

