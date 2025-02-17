#ifndef SCRIPTSENTENCE_H
#define SCRIPTSENTENCE_H

#include <vector>
#include <string>

namespace argosServer {

  class ScriptManager;
  class ScriptFunction;
  class Script;
  class Communicator;

  class ScriptSentence {
  public:
    ScriptSentence(const std::string& scriptSentence);
    ~ScriptSentence();

    void execute(Script& owner, const ScriptManager& scriptManager, Communicator& com);

    size_t numArgs() const;
    const std::string& getCommand() const;
    const std::string& getArg(int index) const;
    const std::vector<std::string>& getArgs() const;

    ScriptFunction* getScriptFunction();

    // STL
    ScriptSentence();
    ScriptSentence(const ScriptSentence& sentence);
    ScriptSentence& operator=(const ScriptSentence& sentence);

    friend std::ostream& operator<<(std::ostream& os, const ScriptSentence& scriptSentence);

  private:
    void decouple(const std::string& scriptSentence);

  private:
    ScriptFunction* _sf;
    std::string _command;
    std::vector<std::string> _args;
  };

}

#endif
