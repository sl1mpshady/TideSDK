/**
 * Copyright (c) 2012 - 2014 TideSDK contributors 
 * http://www.tidesdk.org
 * Includes modified sources under the Apache 2 License
 * Copyright (c) 2008 - 2012 Appcelerator Inc
 * Refer to LICENSE for details of distribution and use.
 **/

#include "../tide.h"
#include <sstream>

namespace tide
{

    ArgList::ArgList()
    {
        this->args = new std::vector<ValueRef>;
    }

    ArgList::ArgList(ValueRef a)
    {
        this->args = new std::vector<ValueRef>;
        this->args->push_back(a);
    }

    ArgList::ArgList(ValueRef a, ValueRef b)
    {
        this->args = new std::vector<ValueRef>;
        this->args->push_back(a);
        this->args->push_back(b);
    }

    ArgList::ArgList(ValueRef a, ValueRef b, ValueRef c)
    {
        this->args = new std::vector<ValueRef>;
        this->args->push_back(a);
        this->args->push_back(b);
        this->args->push_back(c);
    }

    ArgList::ArgList(ValueRef a, ValueRef b, ValueRef c, ValueRef d)
    {
        this->args = new std::vector<ValueRef>;
        this->args->push_back(a);
        this->args->push_back(b);
        this->args->push_back(c);
        this->args->push_back(d);
    }

    ArgList::ArgList(const ArgList& other)
    {
        this->args = other.args;
    }

    void ArgList::push_back(ValueRef v)
    {
        this->args->push_back(v);
    }

    size_t ArgList::size() const
    {
        return this->args->size();
    }

    const ValueRef& ArgList::at(size_t index) const
    {
        return this->args->at(index);
    }

    const ValueRef& ArgList::operator[](size_t index) const
    {
        return this->args->at(index);
    }

    std::string ArgList::GenerateSignature(const char* name,
         std::string& argSpec)
    {
        std::string out(name);
        out += "(";;
        std::string::iterator it;
        bool optional = false;
        bool lastCharacterWasComma = false;
        for (it = argSpec.begin(); it < argSpec.end(); it++)
        {
            if (*it == ' ' || *it == ',')
            {
                 if (!lastCharacterWasComma)
                    out += ", ";
                lastCharacterWasComma = true;
                continue;
            }

            lastCharacterWasComma = false;
            switch (*it)
            {
                case '|':
                    out += "|";
                    break;

                case '?':
                    out += "[";
                    optional = true;
                    break;

                case 's':
                    out += "String";
                    break;
                case 'b':
                    out += "Boolean";
                    break;
                case 'i':
                    out += "Integer";
                    break;
                case 'd':
                    out += "Double";
                    break;
                case 'n':
                    out += "Number";
                    break;
                case 'o':
                    out += "Object";
                    break;
                case 'l':
                    out += "Array";
                    break;
                case 'm':
                    out += "Function";
                    break;
                case '0':
                    out += "null";
                    break;
                default:
                    out += "Unknown,";
            }
        }

        if (optional)
            out += "]";

        out += ")";
        return out;
    }

    bool ArgList::Verify(std::string& argSpec) const
    {
        bool optional = false;
        std::string::iterator it;
        size_t i = 0;
        for (it = argSpec.begin(); it < argSpec.end(); it++)
        {
            switch (*it)
            {
                // Ignore any spaces or commas.
                case ' ':
                case ',':
                    break;

                // If we find an OR operator here it means the left side of the OR
                // was true so we can skip it and the next argument type.
                case '|':
                    it++;
                    break;

                // The first time we see the optional parameter we stay in
                // optional mode until the end of the session.
                case '?':
                    optional = true;
                    break;

                default:
                    // Verify argument count is correct.
                    if (this->size() < i + 1)
                    {
                        // we are in optional mode, so arguments are valid
                        if (optional) return true;

                        // missing arguments not optional, invalid arguments
                        else return false;
                    }

                    // Verify argument type
                    if (!ArgList::VerifyArg(this->at(i), *it))
                    {
                        // Check for OR operator following current character
                        if (++it == argSpec.end() || *it != '|')
                        {
                            // No OR operator, so arguments are invalid.
                            return false;
                        }
                        else
                        {
                            // OR operator is next, so do another loop
                            // but not increment the count.
                            continue;
                        }
                    }

                    // Increment the argument counter.
                    i++;
            }
        }
        return true;
    }

    void ArgList::VerifyException(const char *name, std::string argSpec) const
    {
        
        if (!Verify(argSpec))
        {
            std::string signature(GenerateSignature(name, argSpec));
            throw ValueException::FromFormat("Invalid arguments passed for: %s",
                 signature.c_str());
        }
    }

    inline bool ArgList::VerifyArg(ValueRef arg, char t)
    {
        if ((t == 's' && arg->IsString())
         || (t == 'b' && arg->IsBool())
         || (t == 'i' && arg->IsInt())
         || (t == 'd' && arg->IsDouble())
         || (t == 'n' && arg->IsNumber())
         || (t == 'o' && arg->IsObject())
         || (t == 'l' && arg->IsList())
         || (t == 'm' && arg->IsMethod())
         || (t == '0' && (arg->IsNull() || arg->IsUndefined())))
            return true;
        else
            return false;
    }

    ValueRef ArgList::GetValue(size_t index, ValueRef defaultValue) const
    {
        if (this->size() > index)
        {
            return this->at(index);
        }
        else
        {
            return defaultValue;
        }

    }

    int ArgList::GetInt(size_t index, int defaultValue) const
    {
        if (this->size() > index && this->at(index)->IsInt())
        {
            return this->at(index)->ToInt();
        }
        else
        {
            return defaultValue;
        }
    }

    double ArgList::GetDouble(size_t index, double defaultValue) const
    {
        if (this->size() > index && this->at(index)->IsDouble())
        {
            return this->at(index)->ToDouble();
        }
        else
        {
            return defaultValue;
        }
    }

    double ArgList::GetNumber(size_t index, double defaultValue) const
    {
        if (this->size() > index && this->at(index)->IsNumber())
        {
            return this->at(index)->ToDouble();
        }
        else
        {
            return defaultValue;
        }
    }

    bool ArgList::GetBool(size_t index, bool defaultValue) const
    {
        if (this->size() > index && this->at(index)->IsBool())
        {
            return this->at(index)->ToBool();
        }
        else
        {
            return defaultValue;
        }
    }

    std::string ArgList::GetString(size_t index, std::string defaultValue) const
    {
        if (this->size() > index && this->at(index)->IsString())
        {
            return this->at(index)->ToString();
        }
        else
        {
            return defaultValue;
        }
    }

    TiObjectRef ArgList::GetObject(size_t index, TiObjectRef defaultValue) const
    {
        if (this->size() > index && this->at(index)->IsObject())
        {
            return this->at(index)->ToObject();
        }
        else
        {
            return defaultValue;
        }
    }

    TiMethodRef ArgList::GetMethod(size_t index, TiMethodRef defaultValue) const
    {
        if (this->size() > index && this->at(index)->IsMethod())
        {
            return this->at(index)->ToMethod();
        }
        else
        {
            return defaultValue;
        }
    }

    TiListRef ArgList::GetList(size_t index, TiListRef defaultValue) const
    {
        if (this->size() > index && this->at(index)->IsList())
        {
            return this->at(index)->ToList();
        }
        else
        {
            return defaultValue;
        }
    }
}
