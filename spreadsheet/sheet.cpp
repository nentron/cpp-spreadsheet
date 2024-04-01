#include "sheet.h"

#include "cell.h"
#include "common.h"

using namespace std::literals;


Sheet::~Sheet() = default;

void Sheet::InvalidCachePos(Position pos){
    if (depended_cells_.count(pos) != 0){
        InvalidateCache(depended_cells_.at(pos));
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

bool Sheet::SuccessSet(std::unique_ptr<Cell>& cell, Position pos, std::string text){
    const std::vector<Position> old_reff_pos = cell-> GetReferencedCells();
    try {
        cell -> Set(std::move(text));
    } catch (const ParsingError&){
        return false;
    }
    return true;
}

void Sheet::SetCell(Position pos, std::string text){
    CheckPosValidation(pos);

    if (table_.count(pos) != 0){
        auto& cell = table_[pos];
        if ((cell -> GetImpl() && cell -> GetText() != text)){
            if (!SuccessSet(cell, pos, std::move(text))){
                return ;
            }

        }
    } else {
        std::unique_ptr<Cell> cell = std::make_unique<Cell>(*this);
        cell -> SetPos(pos);
        if (SuccessSet(cell, pos, std::move(text))){
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
            table_[pos] -> Clear();
            table_.erase(pos);

            if ((pos.col == cols_ - 1 && pos.row < rows_)
                || (pos.row == rows_ - 1 && pos.col < cols_)){
                ReducePrintableSize();
            }
        }
    }
}

void Sheet::PrintValues(std::ostream& out) const {
    for (int row = 0; row < rows_; ++row){
        for (int col = 0; col < cols_; ++col){
            if (table_.count({row, col}) != 0){
                std::visit(
                    [&out](auto&& element){out << element;},
                    table_.at({row, col}) -> GetValue()
                );
            }
            if (col < cols_ - 1){
                out << '\t';
            }
        }
        out << '\n';
    }
}


void Sheet::PrintTexts(std::ostream& out) const{
    for (int row = 0; row < rows_; ++row){
        for (int col = 0; col < cols_; ++col){
            if (table_.count({row, col}) != 0){
                out << table_.at({row, col}) -> GetText();
            }
            if (col < cols_ - 1){
                out << '\t';
            }
        }
        out << '\n';
    }
}

std::unique_ptr<SheetInterface> CreateSheet(){
    return std::make_unique<Sheet>();
}