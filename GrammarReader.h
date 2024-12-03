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
    R"(^\s*)" + NON_TERM + TO + TERM + R"(\s*$)";

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

inline std::optional<bool> GetTypeOfGrammar(const std::string& line)
{
    if (std::regex_match(line, LEFT_HAND_GRAMMAR_REG))
    {
        return true;
    }

    if (std::regex_match(line, RIGHT_HAND_GRAMMAR_REG))
    {
        return false;
    }

    return std::nullopt;
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

inline bool IsCorrectGrammar(const std::string& line, std::optional<bool>& isLeftHandGrammar)
{
    if (IsOnlyWithTermRule(line))
    {
        return true;
    }

    if (isLeftHandGrammar.value() == IsLeftHandRule(line)
        || !isLeftHandGrammar.value() == IsRightHandRule(line))
    {
        return true;
    }

    return false;
}

inline std::string GetOneRule(std::istream& input, std::optional<bool>& isLeftHandGrammar, std::string& line)
{
    if (!isLeftHandGrammar.has_value())
    {
        isLeftHandGrammar = GetTypeOfGrammar(line);
    }

    if (isLeftHandGrammar.has_value() && IsCorrectGrammar(line, isLeftHandGrammar))
    {
        return line;
    }

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

    if (!isLeftHandGrammar.has_value())
    {
        isLeftHandGrammar = GetTypeOfGrammar(line);
    }

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

inline void ParseLeftHandGrammarLine(std::stringstream& line, MooreAutomata& automata)
{
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

inline void ParseRightHandGrammarLine(std::stringstream& line, MooreAutomata& automata)
{
    if (automata.IsEmpty())
    {
        automata.AddState(BASE_STATE);
        automata.SetFinalState(BASE_STATE);
    }

    std::string curNoterm;
    line >> curNoterm;

    if (automata.GetStartState().empty())
    {
        automata.SetStartState(curNoterm);
    }

    automata.AddState(curNoterm);

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
            else //if (next == "|" || next.empty())
            {
                automata.AddTransition(curNoterm, data, BASE_STATE);
            }

            automata.AddInputSymbol(data);
        }
    }
}

inline void ParseLine(MooreAutomata& automata, bool isLeftHandGrammar, const std::string& line)
{
    std::stringstream ss(line);

    if (isLeftHandGrammar)
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

        if (!isLeftHandGrammar.has_value())
        {
            throw std::invalid_argument("Incorrect type of grammar");
        }

        ParseLine(automata, isLeftHandGrammar.value(), line);
    }
}

inline MooreAutomata GetAutomata(std::ifstream& input)
{
    MooreAutomata automata;

    ParseLines(automata, input);

    return automata;
}
