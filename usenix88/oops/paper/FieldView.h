#ifndef FIELDVIEWH
#define FIELDVIEWH

class FieldView: public Object {
	Field* field;
public:
	FieldView(const Field& f) {
		field = &f;
		f->addDependent(*this);
	}
	FieldView~()		{ p->removeDependent(*this); }
};

int NumberField::value(int newval) {
	val = newval;
	changed();
	return val;
}

void NumberFieldView::update(Object& fld, Object& /*unused*/)
{
//	use f->location(), f->value() to update view of field.
}

#endif
