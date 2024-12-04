#pragma once
#include <map>
#include <set>
#include <string>
#include <vector>

struct Transitions
{
    void AddTransition(const std::string& term, const std::string& noterm)
    {
        transitions[term].push_back(noterm);
    }

    std::map<std::string, std::vector<std::string>> transitions; // term -> non term
};

class MooreAutomata
{
public:

    [[nodiscard]] bool IsEmpty() const
    {
        return m_states.empty();
    }

    [[nodiscard]] std::string GetFinalState() const
    {
        return m_finalState;
    }

    void SetStartState(const std::string& state)
    {
        m_startState = state;
    }

    [[nodiscard]] std::string GetStartState() const
    {
        return m_startState;
    }

    void SetFinalState(const std::string& state)
    {
        m_finalState = state;
    }

    void AddState(const std::string& state)
    {
        m_states.insert(state);
    }

    void AddInputSymbol(const std::string& input)
    {
        m_inputs.insert(input);
    }

    void AddTransition(const std::string& state, const std::string& term, const std::string& nextState)
    {
        m_statesTransitions[state].AddTransition(term, nextState);
    }

    [[nodiscard]] std::vector<std::string> GetUndefinedLines() const
    {
        return m_undefinedLines;
    }

    void AddUndefinedString(const std::string& line)
    {
        m_undefinedLines.push_back(line);
    }

    void ClearUndefinedLines()
    {
        m_undefinedLines.clear();
    }

    void ExportToFile(const std::string& outputFilename)
    {
        std::ofstream file(outputFilename);
        if (!file.is_open())
        {
            throw std::runtime_error("Could not open file " + outputFilename);
        }

        SetNewStateNames();

        std::string outputs = ";";
        std::string states = ";";

        for (auto& state: m_states)
        {
            states += state;

            if (state == m_finalState)
            {
                outputs += "F";
            }
            else
            {
                outputs += ";";
                states += ";";
            }
        }

        file << outputs << std::endl;
        file << states << std::endl;

        for (auto& input: m_inputs)
        {
            file << input << ";";
            for (int i = 1; auto& state: m_states)
            {
                if (m_statesTransitions.contains(state) &&
                    m_statesTransitions.at(state).transitions.contains(input))
                {
                    auto transitions = m_statesTransitions.at(state).transitions.at(input);

                    for (int j = 0; j < transitions.size(); ++j)
                    {
                        file << transitions.at(j);
                        if (j != transitions.size() - 1)
                        {
                            file << ",";
                        }
                    }
                    if (i != m_states.size())
                    {
                        file << ";";
                    }
                }
                else if (i != m_states.size())
                {
                    file << ";";
                }
                ++i;
            }
            file << "\n";
        }

        file.close();
    }

private:
    std::vector<std::string> m_undefinedLines;

    void SetNewStateNames()
    {
        std::map<std::string, Transitions> newStatesTransitions;

        auto oldStatesToNewStates = GetNewStateNames();
        std::set<std::string> newStates = GetNewStates(oldStatesToNewStates);

        for (auto& it: m_statesTransitions)
        {
            std::pair<std::string, Transitions> row;
            row.first = oldStatesToNewStates.at(it.first);

            for (auto& transitions: it.second.transitions)
            {
                std::pair<std::string, std::vector<std::string>> transitionsRow;
                transitionsRow.first = transitions.first;

                for (auto& state: transitions.second)
                {
                    transitionsRow.second.push_back(oldStatesToNewStates.at(state));
                }

                row.second.transitions.insert(transitionsRow);
            }

            newStatesTransitions.insert(row);
        }

        m_states = newStates;
        m_statesTransitions = newStatesTransitions;
        m_finalState = oldStatesToNewStates.at(m_finalState);
    }

    static constexpr char NEW_STATE_CHAR = 'q';

    [[nodiscard]] std::map<std::string, std::string> GetNewStateNames() const
    {
        std::map<std::string, std::string> newStateNames;

        unsigned i = 1;
        for (auto& state : m_states)
        {
            if (state == m_finalState)
            {
                newStateNames[m_finalState] = NEW_STATE_CHAR + std::to_string(m_states.size() - 1);
                continue;
            }

            if (state == m_startState)
            {
                newStateNames[m_startState] = NEW_STATE_CHAR + std::to_string(0);
                continue;
            }

            newStateNames[state] = NEW_STATE_CHAR + std::to_string(i++);
        }
        //
        // for (auto& it: newStateNames)
        // {
        //     std::cout << it.first << ": " << it.second << std::endl;
        // }

        return newStateNames;
    }

    static std::set<std::string> GetNewStates(std::map<std::string, std::string>& newStateNames)
    {
        std::set<std::string> newStates;
        for (auto& state: newStateNames)
        {
            newStates.insert(state.second);
        }

        return newStates;
    }

    std::set<std::string> m_states;
    std::set<std::string> m_inputs;
    std::map<std::string, Transitions> m_statesTransitions;
    std::string m_finalState = "";
    std::string m_startState = "";
};
