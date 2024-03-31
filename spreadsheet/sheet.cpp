#include "sheet.h"

#include "cell.h"
#include "common.h"

using namespace std::literals;


Sheet::~Sheet() = default;


void Sheet::CheckCircularDependency(Position start_pos, const std::vector<Position>& reff_cells){
    for (const auto& pos : reff_cells){
        if (start_pos == pos){
            throw CircularDependencyException{"Circule in reff cells"s};
        }
        table_.count(pos) != 0 ? CheckCircularDependency(
            start_pos, table_.at(pos) -> GetReferencedCells()
        ) : CheckCircularDependency(start_pos, {});
    }
}

void Sheet::InvalidateCache(const DependedCells& depended_cells){
    for (auto pos : depended_cells){
        if (table_.count(pos) != 0){
            table_[pos] -> ResetCache();
        }
        depended_cells_.count(pos) != 0
                ? InvalidateCache(depended_cells_[pos]) : InvalidateCache({});
    }
}

void Sheet::CheckPosValidation(Position pos) const {
    if (!pos.IsValid()){
        throw InvalidPositionException{"Wrong cell position, out of table"s};
    }
}

void Sheet::RemoveOldDependedCells(Position cell, const std::vector<Position>& cells){
    for (auto pos : cells){
        if (depended_cells_.count(pos) != 0) {
            depended_cells_[pos].erase(cell);
        }
    }
}

void Sheet::AddNewDependedCells(Position cell, const std::vector<Position>& cells){
    for (const auto& pos : cells){
        depended_cells_[pos].insert(cell);
    }
}

bool Sheet::SetImpl(std::unique_ptr<Cell>& cell, Position pos, std::string text){
    const std::vector<Position> old_reff_pos = cell-> GetReferencedCells();
    if (text[0] == FORMULA_SIGN && text.size() > 1){
        try {
            std::unique_ptr<Impl> temp = std::make_unique<FormulaImpl>(text.substr(1), *this);
            CheckCircularDependency(pos, temp -> GetReferencedCells());
            cell -> Set(std::move(temp));
        } catch (const ParsingError&)
        {
            return false;
        }
    } else if (!text.empty()){
        cell -> Set(std::make_unique<TextImpl>(std::move(text)));
    } else {
        cell -> Set(std::make_unique<EmptyImpl>());
    }
    cell -> ResetCache();
    if (depended_cells_.count(pos) != 0){
        InvalidateCache(depended_cells_.at(pos));
    }
    RemoveOldDependedCells(pos, old_reff_pos);
    AddNewDependedCells(pos, cell -> GetReferencedCells());

    return true;
}

void Sheet::SetCell(Position pos, std::string text){
    CheckPosValidation(pos);

    if (table_.count(pos) != 0){
        auto& cell = table_[pos];
        if ((cell -> GetImpl() && cell -> GetText() != text)){
            SetImpl(cell, pos, std::move(text));
        }
    } else {
        std::unique_ptr<Cell> cell = std::make_unique<Cell>();
        if (SetImpl(cell, pos, std::move(text))){
            cell -> SetPos(pos);
            table_[pos] = std::move(cell);
        } else {
            return;
        }
    }

    rows_ = pos.row + 1 > rows_ ? pos.row + 1 : rows_;
    cols_ = pos.col + 1 > cols_ ? pos.col + 1 : cols_;
}


const CellInterface* Sheet::GetCell(Position pos) const {
    CheckPosValidation(pos);

    return table_.count(pos) != 0 ? table_.at(pos).get() 
        : pos.col < cols_ && pos.row < rows_ ? EmptyCell.get() : nullptr;
}

CellInterface* Sheet::GetCell(Position pos){
    CheckPosValidation(pos);

    return table_.count(pos) != 0 ? table_[pos].get() 
        : pos.col < cols_ && pos.row < rows_ ? EmptyCell.get() : nullptr;
}

void Sheet::ReducePrintableSize(){
    int max_row = -1;
    int max_col = -1;
    for (int r = rows_ - 1; 0 <= r; --r){
        for (int c = cols_ - 1; c >= 0; --c){
            if (table_.count({r, c}) != 0){
                max_row = r > max_row ? r : max_row;
                max_col = max_col < c ? c : max_col;
            }
        }
    }

    rows_ = max_row + 1;
    cols_ = max_col + 1;
}

Size Sheet::GetPrintableSize() const {
    return {rows_, cols_};
}

void Sheet::ClearCell(Position pos){
    CheckPosValidation(pos);

    if (pos.row < rows_ && pos.col < cols_){
        if (table_.count(pos) != 0){
            RemoveOldDependedCells(pos, table_.at(pos) -> GetReferencedCells());
            table_.erase(pos);

            if ((pos.col == cols_ - 1 && pos.row < rows_)
                || (pos.row == rows_ - 1 && pos.col < cols_)){
                ReducePrintableSize();
            }
        }
        if (depended_cells_.count(pos) != 0){
            InvalidateCache(depended_cells_.at(pos));
        }
    }
}

void Sheet::PrintValues(std::ostream& out) const {
    for (int r = 0; r < rows_; ++r){
        for (int c = 0; c < cols_; ++c){
            if (table_.count({r, c}) != 0){
                std::visit(
                    [&out](auto&& element){out << element;},
                    table_.at({r, c}) -> GetValue()
                );
            }
            if (c < cols_ - 1){
                out << '\t';
            }
        }
        out << '\n';
    }
}


void Sheet::PrintTexts(std::ostream& out) const{
    for (int r = 0; r < rows_; ++r){
        for (int c = 0; c < cols_; ++c){
            if (table_.count({r, c}) != 0){
                out << table_.at({r, c}) -> GetText();
            }
            if (c < cols_ - 1){
                out << '\t';
            }
        }
        out << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet(){
    return std::make_unique<Sheet>();
}