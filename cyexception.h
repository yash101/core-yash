#ifndef CYEXCEPTION_H
#define CYEXCEPTION_H
#include <exception>

namespace cy
{
  class Code;
  class CyException;

  class Code
  {
    friend class CyException;
  protected:
    const char* _msg;
    const int _code;
    const char* _sourceFile;
    const unsigned long long _lineNumber;
  public:
    inline Code() :
      _msg(""),
      _code(0),
      _sourceFile(__FILE__),
      _lineNumber(__LINE__)
    {}

    inline Code(const char* msg, const int code, const char* sourceFile, const unsigned long long lineNumber) :
      _msg(msg),
      _code(code),
      _sourceFile(sourceFile),
      _lineNumber(lineNumber)
    {}

    inline virtual ~Code()
    {}

    inline const char* message()
    {
      return _msg;
    }

    inline int code()
    {
      return _code;
    }

    inline const char* source()
    {
      return _sourceFile;
    }

    inline unsigned long long line()
    {
      return _lineNumber;
    }
  };

  class CyException : public std::exception
  {
  private:
    const char* _msg;
    const Code _code;
  public:
    inline CyException() :
      _msg("A CyException was thrown!"),
      _code(
        "A CyException was thrown!",
        0,
        __FILE__,
        __LINE__
      )
    {}

    inline CyException(Code code) :
      _msg(code._msg),
      _code(code)
    {}

    inline const Code getCode()
    {
      return _code;
    }

    inline virtual const char* what() const throw()
    {
      return _msg;
    }

    inline virtual ~CyException()
    {}
  };
}

#define CODE(msg, code) (cy::Code(msg, code, __FILE__, __LINE__))
#define CYEXCEPTION(msg, code) (cy::CyException(cy::Code(msg, code, __FILE__, __LINE__)))

#endif // CYEXCEPTION_H
