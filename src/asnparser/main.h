#ifndef _MAIN_H
#define _MAIN_H
#include <set>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <sstream>
#include <algorithm>
extern unsigned lineNumber;
extern string  fileName;
extern FILE * yyin;
extern int yyparse();

void yyerror(char * str);


/////////////////////////////////////////
//
//  standard error output from parser
//

enum StdErrorType { Warning, Fatal };

class StdError {
  public:
    StdError(StdErrorType ne) : e(ne) { }
    //StdError(StdErrorType ne, unsigned ln) : e(ne), l(ln) { }
    friend ostream & operator<<(ostream & out, const StdError & e);

  protected:
    StdErrorType e;
    //unsigned     l;
};


/////////////////////////////////////////
//
//  intermediate structures from parser
//


class NamedNumber : public PObject
{
    PCLASSINFO(NamedNumber, PObject);
  public:
    NamedNumber(string * nam);
    NamedNumber(string * nam, int num);
    NamedNumber(string * nam, const string & ref);
    void PrintOn(ostream &) const;

    void SetAutoNumber(const NamedNumber & prev);
    const string & GetName() const { return name; }
    const string & getReference ( ) const { return reference; }
    int GetNumber() const { return number; }
    bool getAutonumber ( ) const { return autonumber; }
  protected:
    string name;
    string reference;
    int number;
    BOOL autonumber;
};

PLIST(NamedNumberList, NamedNumber);


// Types

class TypeBase;

PLIST(TypesList, TypeBase);
PSORTED_LIST(SortedTypesList, TypeBase);

class Tag : public PObject
{
    PCLASSINFO(Tag, PObject);
  public:
    enum Type {
      Universal,
      Application,
      ContextSpecific,
      Private
    };
    enum UniversalTags {
      IllegalUniversalTag,
      UniversalBoolean,
      UniversalInteger,
      UniversalBitString,
      UniversalOctetString,
      UniversalNull,
      UniversalObjectId,
      UniversalObjectDescriptor,
      UniversalExternalType,
      UniversalReal,
      UniversalEnumeration,
      UniversalEmbeddedPDV,
      UniversalSequence = 16,
      UniversalSet,
      UniversalNumericString,
      UniversalPrintableString,
      UniversalTeletexString,
      UniversalVideotexString,
      UniversalIA5String,
      UniversalUTCTime,
      UniversalGeneralisedTime,
      UniversalGraphicString,
      UniversalVisibleString,
      UniversalGeneralString,
      UniversalUniversalString,
      UniversalBMPString = 30
    };
    enum Mode {
      Implicit,
      Explicit,
      Automatic
    };
    Tag(unsigned tagNum);

    void PrintOn(ostream &) const;

    Type type;
    unsigned number;
    Mode mode;

    static const char * classNames[];
    static const char * modeNames[];
};


class ConstraintElementBase;

PLIST(ConstraintElementList, ConstraintElementBase);


class Constraint : public PObject
{
    PCLASSINFO(Constraint, PObject);
  public:
    Constraint(ConstraintElementBase * elmt);
    Constraint(ConstraintElementList * std, BOOL extend, ConstraintElementList * ext);

    void PrintOn(ostream &) const;

    BOOL IsExtendable() const { return extendable; }
    void GenerateCplusplus(const string & fn, ostream & hdr, ostream & cxx);
    BOOL ReferencesType(const TypeBase & type);
    void checkConstraints ( char typ, TypeBase * t );
  protected:
    ConstraintElementList standard;
    BOOL                  extendable;
    ConstraintElementList extensions;
};

typedef vector < Constraint > ConstraintList;

class ConstraintElementBase : public PObject
{
    PCLASSINFO(ConstraintElementBase, PObject);
  public:
    ConstraintElementBase();
    void SetExclusions(ConstraintElementBase * excl) { exclusions = excl; }

    virtual void GenerateCplusplus(const string & fn, ostream & hdr, ostream & cxx);
    virtual BOOL ReferencesType(const TypeBase & type);
    virtual void checkConstraints ( char typ, bool e, TypeBase * t );
  protected:
    ConstraintElementBase * exclusions;
};


class ConstrainAllConstraintElement : public ConstraintElementBase
{
    PCLASSINFO(ConstrainAllConstraintElement, ConstraintElementBase);
  public:
    ConstrainAllConstraintElement(ConstraintElementBase * excl);
    void checkConstraints ( char typ, bool e, TypeBase * t );
};



class ElementListConstraintElement : public ConstraintElementBase
{
    PCLASSINFO(ElementListConstraintElement, ConstraintElementBase);
  public:
    ElementListConstraintElement(ConstraintElementList * list);
    void PrintOn(ostream &) const;

    virtual void GenerateCplusplus(const string & fn, ostream & hdr, ostream & cxx);
    virtual BOOL ReferencesType(const TypeBase & type);
    void checkConstraints ( char typ, bool e, TypeBase * t );
  protected:
    ConstraintElementList elements;
};


class ValueBase;

class SingleValueConstraintElement : public ConstraintElementBase
{
    PCLASSINFO(SingleValueConstraintElement, ConstraintElementBase);
  public:
    SingleValueConstraintElement(ValueBase * val);
    ~SingleValueConstraintElement();
    void PrintOn(ostream &) const;

    virtual void GenerateCplusplus(const string & fn, ostream & hdr, ostream & cxx);
    void checkConstraints ( char typ, bool e, TypeBase * t );
  protected:
    ValueBase * value;
};


class ValueRangeConstraintElement : public ConstraintElementBase
{
    PCLASSINFO(ValueRangeConstraintElement, ConstraintElementBase);
  public:
    ValueRangeConstraintElement(ValueBase * lowerBound, ValueBase * upperBound);
    ~ValueRangeConstraintElement();
    void PrintOn(ostream &) const;

    virtual void GenerateCplusplus(const string & fn, ostream & hdr, ostream & cxx);
    void checkConstraints ( char typ, bool e, TypeBase * t );
  protected:
    ValueBase * lower;
    ValueBase * upper;
};


class SubTypeConstraintElement : public ConstraintElementBase
{
    PCLASSINFO(SubTypeConstraintElement, ConstraintElementBase);
  public:
    SubTypeConstraintElement(TypeBase * typ);
    ~SubTypeConstraintElement();
    void PrintOn(ostream &) const;
    void GenerateCplusplus(const string &, ostream &, ostream &);
    virtual BOOL ReferencesType(const TypeBase & type);
    void checkConstraints ( char typ, bool e, TypeBase * t );
  protected:
    TypeBase * subtype;
};


class NestedConstraintConstraintElement : public ConstraintElementBase
{
    PCLASSINFO(NestedConstraintConstraintElement, ConstraintElementBase);
  public:
    NestedConstraintConstraintElement(Constraint * con);
    ~NestedConstraintConstraintElement();

    virtual BOOL ReferencesType(const TypeBase & type);
    void checkConstraints ( char typ, bool e, TypeBase * t );

  protected:
    Constraint * constraint;
};


class SizeConstraintElement : public NestedConstraintConstraintElement
{
    PCLASSINFO(SizeConstraintElement, NestedConstraintConstraintElement);
  public:
    SizeConstraintElement(Constraint * constraint);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(const string & fn, ostream & hdr, ostream & cxx);
    void checkConstraints ( char typ, bool e, TypeBase * t );
};


class FromConstraintElement : public NestedConstraintConstraintElement
{
    PCLASSINFO(FromConstraintElement, NestedConstraintConstraintElement);
  public:
    FromConstraintElement(Constraint * constraint);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(const string & fn, ostream & hdr, ostream & cxx);
    void checkConstraints ( char typ, bool e, TypeBase * t );
};


class WithComponentConstraintElement : public NestedConstraintConstraintElement
{
    PCLASSINFO(WithComponentConstraintElement, NestedConstraintConstraintElement);
  public:
    WithComponentConstraintElement(string * name, Constraint * constraint, int presence);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(const string & fn, ostream & hdr, ostream & cxx);

    enum {
      Present,
      Absent,
      Optional,
      Default
    };

    void checkConstraints ( char typ, bool e, TypeBase * t );
  protected:
    string name;
    int     presence;
};


class InnerTypeConstraintElement : public ElementListConstraintElement
{
    PCLASSINFO(InnerTypeConstraintElement, ElementListConstraintElement);
  public:
    InnerTypeConstraintElement(ConstraintElementList * list, BOOL partial);

    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(const string & fn, ostream & hdr, ostream & cxx);
    void checkConstraints ( char typ, bool e, TypeBase * t );

  protected:
    BOOL partial;
};


class UserDefinedConstraintElement : public ConstraintElementBase
{
    PCLASSINFO(UserDefinedConstraintElement, ConstraintElementBase);
  public:
    UserDefinedConstraintElement(TypesList * types);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(const string & fn, ostream & hdr, ostream & cxx);
    void checkConstraints ( char typ, bool e, TypeBase * t );
  protected:
    TypesList types;
};



class TypeBase : public PObject
{
    PCLASSINFO(TypeBase, PObject);
  public:
    Comparison Compare(const PObject & obj) const;
    void PrintOn(ostream &) const;

    virtual int GetIdentifierTokenContext() const;
    virtual int GetBraceTokenContext() const;

    const string & GetName() const { return name; }
    void SetName(string * name);
    string GetIdentifier() const { return identifier; }
    void SetTag(Tag::Type cls, unsigned num, Tag::Mode mode);
    const Tag & GetTag() const { return tag; }
    BOOL HasNonStandardTag() const { return tag != defaultTag; }
    void SetParameters(list<string> * list);
    void AddConstraint(Constraint * constraint) { constraints.push_back(*constraint); }
    BOOL HasConstraints() const { return constraints.size() > 0; }
    void MoveConstraints(TypeBase * from);
    BOOL HasParameters() const { return !parameters.empty(); }
    BOOL IsOptional() const { return isOptional; }
    void SetOptional() { isOptional = TRUE; }
    void SetDefaultValue(ValueBase * value) { defaultValue = value; }
    const string & GetTemplatePrefix() const { return templatePrefix; }
    const string & GetClassNameString() const { return classNameString; }

    virtual void AdjustIdentifier( bool useNamespace );
    virtual void FlattenUsedTypes();
    virtual TypeBase * FlattenThisType(const TypeBase & parent);
    virtual BOOL IsChoice() const;
    virtual BOOL IsParameterizedType() const;
    virtual BOOL IsPrimitiveType() const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateForwardDecls(ostream & hdr);
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const TypeBase & actualType);
    virtual const char * GetAncestorClass() const = 0;
    virtual string GetTypeName() const;
    virtual string defaultParameters ( ) const;
    virtual string defaultExplicit ( ) const;
    virtual BOOL CanReferenceType() const;
    virtual BOOL ReferencesType(const TypeBase & type);
    virtual BOOL ReferencesTypeInside(const TypeBase & type);
    virtual void SetImportPrefix(const string &);
    virtual BOOL IsParameterisedImport() const;
    virtual bool isAsnStandardClass ( ) const;
    virtual void noInlines ( );

    BOOL IsGenerated() const { return isGenerated; }
    void BeginGenerateCplusplus(ostream & hdr, ostream & cxx);
    void EndGenerateCplusplus(ostream & hdr, ostream & cxx);
    void GenerateCplusplusConstructor(ostream & hdr, ostream & cxx);
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    void addConstraint ( char typ, bool e, const ConstraintElementBase * c );
    void checkConstraints ( char typ );
    virtual TypeBase * Clone ( ) const = 0;
    virtual bool requiresModuleInclude ( const string & m );
    virtual string getImportPrefix ( ) const;
  protected:
    TypeBase(unsigned tagNum);
    TypeBase(TypeBase * copy);

    void PrintStart(ostream &) const;
    void PrintFinish(ostream &) const;

    string        name;
    string        identifier;
    Tag            tag;
    Tag            defaultTag;
    ConstraintList constraints;
    BOOL           isOptional;
    ValueBase    * defaultValue;
    BOOL           isGenerated;
    list<string>    parameters;
    string        templatePrefix;
    string        classNameString;
    struct CheckedConstraint {
	const ConstraintElementBase * c;
	bool e;
	CheckedConstraint ( const ConstraintElementBase * cc, bool ee ) : c ( cc ), e ( ee ) { }
	CheckedConstraint ( ) { }
    };
    vector < CheckedConstraint > fromConstraints, sizeConstraints, typeConstraints, simpleConstraints;
};


class DefinedType : public TypeBase
{
    PCLASSINFO(DefinedType, TypeBase);
  public:
    DefinedType(string * name, BOOL parameter);
    DefinedType(TypeBase * refType, TypeBase * bType);
    DefinedType(TypeBase * refType, const string & name);
    DefinedType(TypeBase * refType, const TypeBase & parent);

    void PrintOn(ostream &) const;

    virtual BOOL IsChoice() const;
    virtual BOOL IsParameterizedType() const;
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const TypeBase & actualType);
    virtual const char * GetAncestorClass() const;
    virtual string GetTypeName() const;
    virtual BOOL CanReferenceType() const;
    virtual BOOL ReferencesType(const TypeBase & type);
    virtual BOOL ReferencesTypeInside(const TypeBase & type);
    virtual bool isAsnStandardClass ( ) const;
    virtual TypeBase * FlattenThisType(const TypeBase & parent);
    virtual TypeBase * Clone ( ) const;
    virtual void noInlines ( );
    bool requiresModuleInclude ( const string & m );
    
  protected:
    void ConstructFromType(TypeBase * refType, const string & name);
    void resolve ( );
    string referenceName;
    TypeBase * baseType;
    BOOL unresolved;
};


class ParameterizedType : public DefinedType
{
    PCLASSINFO(ParameterizedType, DefinedType);
  public:
    ParameterizedType(string * name, TypesList * args);

    void PrintOn(ostream &) const;

    virtual BOOL IsParameterizedType() const;
    virtual string GetTypeName() const;
    virtual BOOL ReferencesType(const TypeBase & type);
    virtual BOOL ReferencesTypeInside(const TypeBase & type);
    virtual TypeBase * FlattenThisType(const TypeBase & parent);
    virtual TypeBase * Clone ( ) const;

  protected:
    TypesList arguments;
};


class SelectionType : public TypeBase
{
    PCLASSINFO(SelectionType, TypeBase);
  public:
    SelectionType(string * name, TypeBase * base);
    ~SelectionType();

    void PrintOn(ostream &) const;

    virtual void FlattenUsedTypes();
    virtual TypeBase * FlattenThisType(const TypeBase & parent);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual const char * GetAncestorClass() const;
    virtual BOOL CanReferenceType() const;
    virtual BOOL ReferencesType(const TypeBase & type);
    virtual TypeBase * Clone ( ) const;

  protected:
    string selection;
    TypeBase * baseType;
};


class BooleanType : public TypeBase
{
    PCLASSINFO(BooleanType, TypeBase);
  public:
    BooleanType();
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const TypeBase & actualType);
    virtual const char * GetAncestorClass() const;
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual TypeBase * Clone ( ) const;
};


class IntegerType : public TypeBase
{
    PCLASSINFO(IntegerType, TypeBase);
  public:
    IntegerType();
    IntegerType(NamedNumberList *);
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const TypeBase & actualType);
    virtual const char * GetAncestorClass() const;
    virtual string defaultParameters ( ) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual TypeBase * Clone ( ) const;
  protected:
    NamedNumberList allowedValues;
};


class EnumeratedType : public TypeBase
{
    PCLASSINFO(EnumeratedType, TypeBase);
  public:
    EnumeratedType(NamedNumberList * enums, BOOL extend, NamedNumberList * ext);
    void PrintOn(ostream &) const;
    virtual TypeBase * FlattenThisType(const TypeBase & parent);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const TypeBase & actualType);
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
  protected:
    NamedNumberList enumerations;
    PINDEX numEnums;
    BOOL extendable;
};


class RealType : public TypeBase
{
    PCLASSINFO(RealType, TypeBase);
  public:
    RealType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class BitStringType : public TypeBase
{
    PCLASSINFO(BitStringType, TypeBase);
  public:
    BitStringType();
    BitStringType(NamedNumberList *);
    virtual int GetIdentifierTokenContext() const;
    virtual int GetBraceTokenContext() const;
    virtual const char * GetAncestorClass() const;
    virtual string defaultParameters ( ) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual TypeBase * Clone ( ) const;
  protected:
    NamedNumberList allowedBits;
};


class OctetStringType : public TypeBase
{
    PCLASSINFO(OctetStringType, TypeBase);
  public:
    OctetStringType();
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const TypeBase & actualType);
    virtual const char * GetAncestorClass() const;
    virtual string defaultParameters ( ) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual TypeBase * Clone ( ) const;
};


class NullType : public TypeBase
{
    PCLASSINFO(NullType, TypeBase);
  public:
    NullType();
    virtual const char * GetAncestorClass() const;
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual TypeBase * Clone ( ) const;
};


class SequenceType : public TypeBase
{
    PCLASSINFO(SequenceType, TypeBase);
    void PrintOn(ostream &) const;
  public:
    SequenceType(TypesList * std,
                 BOOL extendable,
                 TypesList * extensions,
                 unsigned tagNum = Tag::UniversalSequence);
    virtual void FlattenUsedTypes();
    virtual TypeBase * FlattenThisType(const TypeBase & parent);
    virtual BOOL IsPrimitiveType() const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateForwardDecls(ostream & hdr);
    virtual const char * GetAncestorClass() const;
    virtual BOOL CanReferenceType() const;
    virtual BOOL ReferencesTypeInside(const TypeBase & type);
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual TypeBase * Clone ( ) const;
    virtual bool requiresModuleInclude ( const string & m );
  protected:
    TypesList fields;
    PINDEX numFields;
    BOOL extendable;
};


class SequenceOfType : public TypeBase
{
    PCLASSINFO(SequenceOfType, TypeBase);
  public:
    SequenceOfType(TypeBase * base, Constraint * constraint, unsigned tag = Tag::UniversalSequence);
    ~SequenceOfType();
    void PrintOn(ostream &) const;
    virtual void FlattenUsedTypes();
    virtual TypeBase * FlattenThisType(const TypeBase & parent);
    virtual BOOL IsPrimitiveType() const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateForwardDecls(ostream & hdr);
    virtual const char * GetAncestorClass() const;
    virtual BOOL CanReferenceType() const;
    virtual BOOL ReferencesTypeInside(const TypeBase & type);
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual TypeBase * Clone ( ) const;
  protected:
    TypeBase * baseType;
};


class SetType : public SequenceType
{
    PCLASSINFO(SetType, SequenceType);
  public:
    SetType();
    SetType(SequenceType * seq);
    virtual const char * GetAncestorClass() const;
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual TypeBase * Clone ( ) const;
};


class SetOfType : public SequenceOfType
{
    PCLASSINFO(SetOfType, SequenceOfType);
  public:
    SetOfType(TypeBase * base, Constraint * constraint);
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual TypeBase * Clone ( ) const;
};


class ChoiceType : public SequenceType
{
    PCLASSINFO(ChoiceType, SequenceType);
    bool usingInlines;
  public:
    ChoiceType(TypesList * std = NULL,
               BOOL extendable = FALSE,
               TypesList * extensions = NULL);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateForwardDecls(ostream & hdr);
    virtual BOOL IsPrimitiveType() const;
    virtual BOOL IsChoice() const;
    virtual const char * GetAncestorClass() const;
    virtual BOOL ReferencesTypeInside(const TypeBase & type);
    virtual TypeBase * Clone ( ) const;
    virtual void noInlines ( );
};


class EmbeddedPDVType : public TypeBase
{
    PCLASSINFO(EmbeddedPDVType, TypeBase);
  public:
    EmbeddedPDVType();
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class ExternalType : public TypeBase
{
    PCLASSINFO(ExternalType, TypeBase);
  public:
    ExternalType();
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class AnyType : public TypeBase
{
    PCLASSINFO(AnyType, TypeBase);
  public:
    AnyType(string * ident);
    void PrintOn(ostream & strm) const;
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
  protected:
    string identifier;
};


class StringTypeBase : public TypeBase
{
    PCLASSINFO(StringTypeBase, TypeBase);
  public:
    StringTypeBase(int tag);
    virtual int GetBraceTokenContext() const;
    virtual string defaultParameters ( ) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const TypeBase & actualType);
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
};


class BMPStringType : public StringTypeBase
{
    PCLASSINFO(BMPStringType, StringTypeBase);
  public:
    BMPStringType();
    virtual const char * GetAncestorClass() const;
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const TypeBase & actualType);
    virtual TypeBase * Clone ( ) const;
};


class GeneralStringType : public StringTypeBase
{
    PCLASSINFO(GeneralStringType, StringTypeBase);
  public:
    GeneralStringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class GraphicStringType : public StringTypeBase
{
    PCLASSINFO(GraphicStringType, StringTypeBase);
  public:
    GraphicStringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class IA5StringType : public StringTypeBase
{
    PCLASSINFO(IA5StringType, StringTypeBase);
  public:
    IA5StringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class ISO646StringType : public StringTypeBase
{
    PCLASSINFO(ISO646StringType, StringTypeBase);
  public:
    ISO646StringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class NumericStringType : public StringTypeBase
{
    PCLASSINFO(NumericStringType, StringTypeBase);
  public:
    NumericStringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class PrintableStringType : public StringTypeBase
{
    PCLASSINFO(PrintableStringType, StringTypeBase);
  public:
    PrintableStringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class TeletexStringType : public StringTypeBase
{
    PCLASSINFO(TeletexStringType, StringTypeBase);
  public:
    TeletexStringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class T61StringType : public StringTypeBase
{
    PCLASSINFO(T61StringType, StringTypeBase);
  public:
    T61StringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class UniversalStringType : public StringTypeBase
{
    PCLASSINFO(UniversalStringType, StringTypeBase);
  public:
    UniversalStringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class VideotexStringType : public StringTypeBase
{
    PCLASSINFO(VideotexStringType, StringTypeBase);
  public:
    VideotexStringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class VisibleStringType : public StringTypeBase
{
    PCLASSINFO(VisibleStringType, StringTypeBase);
  public:
    VisibleStringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class UnrestrictedCharacterStringType : public StringTypeBase
{
    PCLASSINFO(UnrestrictedCharacterStringType, StringTypeBase);
  public:
    UnrestrictedCharacterStringType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class GeneralizedTimeType : public TypeBase
{
    PCLASSINFO(GeneralizedTimeType, TypeBase);
  public:
    GeneralizedTimeType();
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class UTCTimeType : public TypeBase
{
    PCLASSINFO(UTCTimeType, TypeBase);
  public:
    UTCTimeType();
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class ObjectDescriptorType : public TypeBase
{
    PCLASSINFO(ObjectDescriptorType, TypeBase);
  public:
    ObjectDescriptorType();
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


class ObjectIdentifierType : public TypeBase
{
    PCLASSINFO(ObjectIdentifierType, TypeBase);
  public:
    ObjectIdentifierType();
    virtual int GetIdentifierTokenContext() const;
    virtual int GetBraceTokenContext() const;
    virtual const char * GetAncestorClass() const;
    virtual void GenerateOperators(ostream & hdr, ostream & cxx, const TypeBase & actualType);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual TypeBase * Clone ( ) const;
};


class ObjectClassFieldType : public TypeBase
{
    PCLASSINFO(ObjectClassFieldType, TypeBase);
  public:
    ObjectClassFieldType(string * objclass, string * field);
    virtual const char * GetAncestorClass() const;
    void PrintOn(ostream &) const;
    virtual TypeBase * FlattenThisType(const TypeBase & parent);
    virtual BOOL IsPrimitiveType() const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual BOOL CanReferenceType() const;
    virtual BOOL ReferencesType(const TypeBase & type);
    virtual TypeBase * Clone ( ) const;
  protected:
    string asnObjectClassName;
    string asnObjectClassField;
};


class ImportedType : public TypeBase
{
    PCLASSINFO(ImportedType, TypeBase);
  public:
    ImportedType(string * name, BOOL parameterised);
    virtual const char * GetAncestorClass() const;
    virtual void AdjustIdentifier( bool useNamespace);
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
    virtual void SetImportPrefix(const string &);
    virtual BOOL IsParameterisedImport() const;
    virtual bool isAsnStandardClass ( ) const;
    virtual TypeBase * Clone ( ) const;
    virtual string getImportPrefix ( ) const;
  protected:
    string modulePrefix;
    BOOL    parameterised;
};


class SearchType : public TypeBase
{
    PCLASSINFO(SearchType, TypeBase);
  public:
    SearchType(const string & name);
    virtual void GenerateCplusplusConstraints(const string & prefix, ostream & hdr, ostream & cxx);
    virtual const char * GetAncestorClass() const;
    virtual TypeBase * Clone ( ) const;
};


// Values

class ValueBase : public PObject
{
    PCLASSINFO(ValueBase, PObject);
  public:
    void SetValueName(string * name);
    const string & GetName() const { return valueName; }

    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);

  protected:
    void PrintBase(ostream &) const;
    string valueName;
};

PLIST(ValuesList, ValueBase);


class DefinedValue : public ValueBase
{
    PCLASSINFO(DefinedValue, ValueBase);
  public:
    DefinedValue(string * name);
    void PrintOn(ostream &) const;
    const string & GetReference() const { return referenceName; }
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
  protected:
    string referenceName;
    ValueBase * actualValue;
    BOOL unresolved;
};


class BooleanValue : public ValueBase
{
    PCLASSINFO(BooleanValue, ValueBase);
  public:
    BooleanValue(BOOL newVal);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
  protected:
    BOOL value;
};


class IntegerValue : public ValueBase
{
    PCLASSINFO(IntegerValue, ValueBase);
  public:
    IntegerValue(PInt64 newVal);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);

    operator PInt64() const { return value; }
    operator long() const { return (long)value; }

  protected:
    PInt64 value;
};


class RealValue : public ValueBase
{
    PCLASSINFO(RealValue, ValueBase);
  public:
    RealValue(double newVal);
  protected:
    double value;
};


class OctetStringValue : public ValueBase
{
    PCLASSINFO(OctetStringValue, ValueBase);
  public:
    OctetStringValue() { }
    OctetStringValue(string * newVal);
  protected:
    PBYTEArray value;
};


class BitStringValue : public ValueBase
{
    PCLASSINFO(BitStringValue, ValueBase);
  public:
    BitStringValue() { }
    BitStringValue(string * newVal);
    BitStringValue(list<string> * newVal);
  protected:
    PBYTEArray value;
};


class NullValue : public ValueBase
{
    PCLASSINFO(NullValue, ValueBase);
};


class CharacterValue : public ValueBase
{
    PCLASSINFO(CharacterValue, ValueBase);
  public:
    CharacterValue(BYTE c);
    CharacterValue(BYTE t1, BYTE t2);
    CharacterValue(BYTE q1, BYTE q2, BYTE q3, BYTE q4);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
  protected:
    unsigned value;
};


class CharacterStringValue : public ValueBase
{
    PCLASSINFO(CharacterStringValue, ValueBase);
  public:
    CharacterStringValue() { }
    CharacterStringValue(string * newVal);
    CharacterStringValue(list<string> * newVal);
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
  protected:
    string value;
};


class ObjectIdentifierValue : public ValueBase
{
    PCLASSINFO(ObjectIdentifierValue, ValueBase);
  public:
    ObjectIdentifierValue(string * newVal);
    ObjectIdentifierValue(list<string> * newVal);
    void PrintOn(ostream &) const;
  protected:
    list<string> value;
};


class MinValue : public ValueBase
{
    PCLASSINFO(MinValue, ValueBase);
  public:
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
};


class MaxValue : public ValueBase
{
    PCLASSINFO(MaxValue, ValueBase);
  public:
    void PrintOn(ostream &) const;
    virtual void GenerateCplusplus(ostream & hdr, ostream & cxx);
};


class SequenceValue : public ValueBase
{
    PCLASSINFO(SequenceValue, ValueBase);
  public:
    SequenceValue(ValuesList * list = NULL);
    void PrintOn(ostream &) const;
  protected:
    ValuesList values;
};


class MibBase : public PObject
{
    PCLASSINFO(MibBase, PObject);
  public:
    MibBase(string * name, string * descr, string * refer, ValueBase * val);
    virtual ~MibBase();
  protected:
    string name;
    string description;
    string reference;
    ValueBase * value;
};

PLIST(MibList, MibBase);


class MibObject : public MibBase
{
    PCLASSINFO(MibObject, MibBase);
  public:
    enum Access {
      read_only,
      read_write,
      write_only,
      not_accessible,
    };
    enum Status {
      mandatory,
      optional,
      obsolete,
      deprecated
    };
    MibObject(string * name, TypeBase * type, Access acc, Status stat,
              string * descr, string * refer, list<string> * idx,
              ValueBase * defVal,
              ValueBase * setVal);
    ~MibObject();
    void PrintOn(ostream &) const;
  protected:
    TypeBase * type;
    Access access;
    Status status;
    list<string> index;
    ValueBase * defaultValue;
};


class MibTrap : public MibBase
{
    PCLASSINFO(MibTrap, MibBase);
  public:
    MibTrap(string * nam, ValueBase * ent, ValuesList * var,
            string * descr, string * refer, ValueBase * val);
    ~MibTrap();
    void PrintOn(ostream &) const;
  protected:
    ValueBase * enterprise;
    ValuesList variables;
};


class ImportModule : public PObject
{
    PCLASSINFO(ImportModule, PObject);
  public:
    ImportModule(string * name, TypesList * syms);

    void PrintOn(ostream &) const;

    void GenerateCplusplus(ostream & hdr, ostream & cxx);

  protected:
    string   fullModuleName;
    string   shortModuleName;
    TypesList symbols;
};

PLIST(ImportsList, ImportModule);
PLIST(DefinedList, DefinedType);

class ModuleDefinition : public PObject
{
    PCLASSINFO(ModuleDefinition, PObject);
  public:
    ModuleDefinition(string * name, list<string> * id, Tag::Mode defTagMode);

    void PrintOn(ostream &) const;

    Tag::Mode GetDefaultTagMode() const { return defaultTagMode; }

    void SetExportAll();
    void SetExports(TypesList * syms);

    void AddImport(ImportModule * mod)  { imports.Append(mod); }
    void AddType(TypeBase * type)       { types.Append(type); }
    void AddValue(ValueBase * val)      { values.Append(val); }
    void AddMIB(MibBase * mib)          { mibs.Append(mib); }

    void AppendType(TypeBase * type);
    TypeBase * FindType(const string & name);
    const ValuesList & GetValues() const { return values; }

    const string & GetModuleName() const { return moduleName; }
    const string & GetPrefix()     const { return classNamePrefix; }

    string GetImportModuleName(const string & moduleName);

    int GetIndentLevel() const { return indentLevel; }
    void SetIndentLevel(int delta) { indentLevel += delta; }

    BOOL UsingInlines() const { return usingInlines; }

    void GenerateCplusplus(const PFilePath & path,
                           const string & modName,
                           unsigned numFiles,
                           BOOL useNamespaces,
                           BOOL useInlines,
                           BOOL verbose);
    bool checkReferencesType ( const PObject * ob );
    bool generateForwardDecl ( const string & typeName );
    bool requiresInclude ( const string & m ) const;
  protected:
    string         moduleName;
    string         classNamePrefix;
    BOOL            separateClassFiles;
    list<string>     definitiveId;
    Tag::Mode       defaultTagMode;
    TypesList       exports;
    BOOL            exportAll;
    ImportsList     imports;
    map<string,string> importNames;
    TypesList       types;
    SortedTypesList sortedTypes;
    ValuesList      values;
    MibList         mibs;
    int             indentLevel;
    BOOL            usingInlines;
    set < const PObject * > crtSet;
    set<string>	    typesOutput;
};


extern ModuleDefinition * Module;


#endif
