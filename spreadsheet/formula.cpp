#include "formula.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <vector>

using namespace std::literals;


FormulaError::FormulaError(Category category)
    : category_(std::move(category))
{}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return this -> category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_){
        case Category::Arithmetic:
            return "#ARITHM!"sv;
        case Category::Ref:
            return "#REF!"sv;
        default:
            return "#VALUE!"sv;
    }
}

std::ostream& operator<<(std::ostream& out, FormulaError err){
    return out << err.ToString();
}

namespace {
    class Formula : public FormulaInterface {
    private:
        FormulaAST ast_;
    public:
        explicit Formula(std::string expression) try 
            : ast_(ParseFormulaAST(expression)){
        } catch (const std::exception& exc) {
            throw FormulaException(exc.what());
        }

        Value Evaluate(const SheetInterface& sheet) const override {
            try {
                return ast_.Execute(sheet);
            } catch (const FormulaError& err){
                return err;
            }
        }

        std::string GetExpression() const override {
            std::ostringstream out;
            ast_.PrintFormula(out);
            return out.str();
        }

        std::vector<Position> GetReferencedCells() const override {
            std::forward_list<Position> cell = ast_.GetCells();
            cell.remove_if([](Position& pos){return !pos.IsValid();});
            cell.unique();
            return {cell.begin(), cell.end()};
        }
    };
}


std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}