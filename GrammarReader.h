#pragma once
#include <fstream>
#include <optional>
#include <string>
#include <sstream>
#include <regex>

#include "Automata/MooreAutomata.h"

const std::string RUS_ALPHABET = "абвгдеёжзийклмнопрстуфхцчшщъыьэюяАБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
const std::string NON_TERM = R"(<[\w)" + RUS_ALPHABET + R"(]+>)";
const std::string TERM = R"([\w)" + RUS_ALPHABET + R"(]|ε)";
const std::string TO = R"(\s*->\s*)";
const std::string OR = R"(\s*\|\s*)";

const std::string LEFT_HAND_GRAMMAR =
    R"(^\s*()" + NON_TERM + R"())" + TO + R"((?:(?:)" + NON_TERM + R"()\s+(?:)" + TERM
    + R"()|)" + TERM + R"()(?:)" + OR + R"((?:)" + NON_TERM + R"(\s+)?(?:)" + TERM + R"())*\s*$)";

const std::string LEFT_HAND_TRANSITION =
    R"(^\s*(?:(?:)" + NON_TERM + R"()\s+(?:)" + TERM + R"()|)" + TERM + R"()(?:)" + OR +
    R"((?:)" + NON_TERM + R"(\s+)?(?:)" + TERM + R"())*\s*(\|)?\s*$)";

const std::string RIGHT_HAND_GRAMMAR =
    R"(^\s*()" + NON_TERM + R"())" + TO + R"(()" + TERM + R"()(?:\s+)" + NON_TERM + R"()?)"
    + R"((?:)" + OR + R"(()" + TERM + R"()(?:\s+)" + NON_TERM + R"()?)*\s*$)";

const std::string RIGHT_HAND_TRANSITION =
    R"(^\s*(?:)" + TERM + R"()(?:\s+)" + NON_TERM + R"()?)"
    + R"((?:)" + OR + R"((?:)" + TERM + R"()(?:\s+)" + NON_TERM + R"()?)*\s*(\|)?\s*$)";

const std::string ONLY_WITH_TERM_GRAMMAR =
    R"(^\s*()" + NON_TERM + R"())" + TO + R"((?:)" + R"((?:)" + TERM
    + R"()|)" + TERM + R"()(?:)" + OR + R"((?:)" + TERM + R"())*\s*$)";

const std::regex LEFT_HAND_GRAMMAR_REG(LEFT_HAND_GRAMMAR);
const std::regex LEFT_HAND_TRANSITION_REG(LEFT_HAND_TRANSITION);
const std::regex RIGHT_HAND_GRAMMAR_REG(RIGHT_HAND_GRAMMAR);
const std::regex RIGHT_HAND_TRANSITION_REG(RIGHT_HAND_TRANSITION);
const std::regex ONLY_WITH_TERM_GRAMMAR_REG(ONLY_WITH_TERM_GRAMMAR);

const std::string BASE_STATE = "H";

inline bool IsTransition(const std::string& line)
{
    if (std::regex_match(line, LEFT_HAND_TRANSITION_REG)
        || std::regex_match(line, RIGHT_HAND_TRANSITION_REG))
    {
        return true;
    }

    return false;
}

inline bool IsEndOfRule(const std::string& line)
{
    std::smatch match;
    if (std::regex_match(line, match, LEFT_HAND_TRANSITION_REG)
        || std::regex_match(line, match, RIGHT_HAND_TRANSITION_REG))
    {
        if (match[1].length() == 0)
        {
            return true;
        }

        return false;
    }

    throw std::invalid_argument("End of rule is not found");
}

inline bool IsLeftHandRule(const std::string& line)
{
    return std::regex_match(line, LEFT_HAND_GRAMMAR_REG);
}

inline bool IsRightHandRule(const std::string& line)
{
    return std::regex_match(line, RIGHT_HAND_GRAMMAR_REG);
}

inline bool IsOnlyWithTermRule(const std::string& line)
{
    return std::regex_match(line, ONLY_WITH_TERM_GRAMMAR_REG);
}

inline void SetTypeOfGrammar(std::optional<bool>& isLeftHandGrammar, const std::string& line)
{
    if (isLeftHandGrammar.has_value() || IsOnlyWithTermRule(line))
    {
        return;
    }

    if (IsLeftHandRule(line))
    {
        isLeftHandGrammar = true;
    }
    else if (IsRightHandRule(line))
    {
        isLeftHandGrammar = false;
    }
}

inline bool IsCorrectGrammar(const std::string& line, std::optional<bool>& isLeftHandGrammar)
{
    if (IsLeftHandRule(line) || IsRightHandRule(line) || IsOnlyWithTermRule(line))
    {
        return true;
    }

    return false;
}

inline void JoinRules(std::istream& input, std::string& line)
{
    while (true)
    {
        std::string nextLine;
        std::getline(input, nextLine);

        if (IsTransition(nextLine))
        {
            line += nextLine;
        }
        else
        {
            throw std::invalid_argument("Invalid transition");
        }

        if (IsEndOfRule(nextLine))
        {
            break;
        }
    }
}

inline std::string GetOneRule(std::istream& input, std::optional<bool>& isLeftHandGrammar, std::string& line)
{
    if (IsCorrectGrammar(line, isLeftHandGrammar))
    {
        SetTypeOfGrammar(isLeftHandGrammar, line);

        return line;
    }

    JoinRules(input, line);

    SetTypeOfGrammar(isLeftHandGrammar, line);

    if (IsCorrectGrammar(line, isLeftHandGrammar))
    {
        return line;
    }

    throw std::invalid_argument("Incorrect grammar");
}

inline bool IsNonTerm(const std::string& str)
{
    return str[0] == '<' && str[str.size() - 1] == '>';
}

inline void ParseLeftHandRule(std::stringstream& line, MooreAutomata& automata, std::string& inNoterm)
{
    std::string data;
    while (line >> data)
    {
        if (data == "->" || data == "|")
        {
            continue;
        }

        if (IsNonTerm(data))
        {
            std::string term;
            line >> term;

            automata.AddState(data);
            automata.AddTransition(data, term, inNoterm);
            automata.AddInputSymbol(term);
        }
        else
        {
            automata.AddTransition(BASE_STATE, data, inNoterm);
            automata.AddInputSymbol(data);
        }
    }
}

inline void ParseUndefinedLinesInLeftHandGrammar(MooreAutomata& automata)
{
    for (auto& line: automata.GetUndefinedLines())
    {
        std::stringstream ss(line);

        std::string inNoterm;
        ss >> inNoterm;

        if (automata.GetFinalState().empty())
        {
            automata.AddState(inNoterm);
            automata.SetFinalState(inNoterm);
        }

        if (automata.GetStartState().empty())
        {
            automata.AddState(BASE_STATE);
            automata.SetStartState(BASE_STATE);
        }

        ParseLeftHandRule(ss, automata, inNoterm);
    }

    automata.ClearUndefinedLines();
}

inline void ParseLeftHandGrammarLine(std::stringstream& line, MooreAutomata& automata)
{
    if (!automata.GetUndefinedLines().empty())
    {
        ParseUndefinedLinesInLeftHandGrammar(automata);
    }

    if (automata.GetStartState().empty())
    {
        automata.SetStartState(BASE_STATE);
    }

    if (automata.IsEmpty())
    {
        automata.AddState(BASE_STATE);
    }

    std::string inNoterm;
    line >> inNoterm;

    automata.AddState(inNoterm);
    if (automata.GetFinalState().empty())
    {
        automata.SetFinalState(inNoterm);
    }

    ParseLeftHandRule(line, automata, inNoterm);
}

inline void ParseRightHandRule(std::stringstream& line, MooreAutomata& automata, std::string& curNoterm)
{
    std::string data;
    while (line >> data)
    {
        if (data == "->" || data == "|")
        {
            continue;
        }

        if (!line.str().empty())
        {
            std::string next;
            line >> next;

            if (IsNonTerm(next))
            {
                automata.AddState(next);
                automata.AddTransition(curNoterm, data, next);
            }
            else
            {
                automata.AddTransition(curNoterm, data, BASE_STATE);
            }

            automata.AddInputSymbol(data);
        }
    }
}

inline void ParseUndefinedLinesInRightHandGrammar(MooreAutomata& automata)
{
    for (auto& line: automata.GetUndefinedLines())
    {
        std::stringstream ss(line);

        std::string curNoterm;
        ss >> curNoterm;

        if (automata.GetStartState().empty())
        {
            automata.SetStartState(curNoterm);
        }

        automata.AddState(curNoterm);

        ParseRightHandRule(ss, automata, curNoterm);
    }

    automata.ClearUndefinedLines();
}

inline void ParseRightHandGrammarLine(std::stringstream& line, MooreAutomata& automata)
{
    if (automata.GetFinalState().empty())
    {
        automata.AddState(BASE_STATE);
        automata.SetFinalState(BASE_STATE);
    }

    if (!automata.GetUndefinedLines().empty())
    {
        ParseUndefinedLinesInRightHandGrammar(automata);
    }

    std::string curNoterm;
    line >> curNoterm;

    if (automata.GetStartState().empty())
    {
        automata.SetStartState(curNoterm);
    }

    automata.AddState(curNoterm);

    ParseRightHandRule(line, automata, curNoterm);
}

inline void ParseLine(MooreAutomata& automata, std::optional<bool>& isLeftHandGrammar, const std::string& line)
{
    std::stringstream ss(line);
    if (!isLeftHandGrammar.has_value())
    {
        automata.AddUndefinedString(line);
    }
    else if (isLeftHandGrammar.value())
    {
        ParseLeftHandGrammarLine(ss, automata);
    }
    else
    {
        ParseRightHandGrammarLine(ss, automata);
    }
}

inline void ParseLines(MooreAutomata& automata, std::istream& input)
{
    std::optional<bool> isLeftHandGrammar;

    while (true)
    {
        std::string line;
        std::getline(input, line);

        if (input.eof())
        {
            break;
        }

        line = GetOneRule(input, isLeftHandGrammar, line);

        ParseLine(automata, isLeftHandGrammar, line);
    }
}

inline MooreAutomata GetAutomata(std::ifstream& input)
{
    MooreAutomata automata;

    ParseLines(automata, input);

    return automata;
}
