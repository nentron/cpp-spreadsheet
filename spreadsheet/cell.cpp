#include "cell.h"
#include "sheet.h"

using namespace std::literals;

// class Cell

CellInterface::Value Cell::GetValue() const {
    if (value_.has_value()){
        return value_.value();
    }
    const CellInterface::Value value = impl_ -> GetValue();
    if (std::holds_alternative<double>(value)){
        value_ = std::get<double>(value);
    }
    return value;
}

void Cell::CheckCircularDependency(const std::vector<Position>& reff_cells){
    for (auto pos : reff_cells){
        if (pos == current_pos_){
            throw CircularDependencyException{"Wrong formula with circular"s};
        }
        const auto cell = sheet_ -> GetCell(pos);
        CheckCircularDependency(cell ? cell -> GetReferencedCells() : std::vector<Position>{});
    }
}

Position& Cell::GetPos(){
    return current_pos_;
}

const Position& Cell::GetPos() const {
    return current_pos_;
}

void Cell::SetPos(Position pos){
    current_pos_ = pos;
}

std::string Cell::GetText() const {
    return impl_ -> GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_ -> GetReferencedCells();
}

const std::unique_ptr<Impl>& Cell::GetImpl() const {
    return impl_;
}

std::unique_ptr<Impl>& Cell::GetImpl(){
    return impl_;
}

void Cell::Set(std::string text){
    const std::vector<Position> old_ref_pos = impl_ ? GetReferencedCells() : std::vector<Position>();
    if (text[0] == FORMULA_SIGN && text.size() > 1){
        std::unique_ptr<Impl> temp = std::make_unique<FormulaImpl>(text.substr(1), *sheet_);
        CheckCircularDependency(temp -> GetReferencedCells());
        impl_ = std::move(temp);
    } else if (!text.empty()){
        impl_ = std::make_unique<TextImpl>(std::move(text));
    } else {
        impl_ = std::make_unique<EmptyImpl>();
    }
    ResetCache();
    sheet_ -> InvalidCachePos(current_pos_);

    sheet_ -> RemoveOldDependedCells(current_pos_, old_ref_pos);
    sheet_ -> AddNewDependedCells(current_pos_, impl_ -> GetReferencedCells());
}

void Cell::Clear(){
    value_.reset();
    sheet_ -> InvalidCachePos(current_pos_);
    sheet_ -> RemoveOldDependedCells(current_pos_, impl_ -> GetReferencedCells());
    impl_.reset();
}

void Cell::ResetCache(){
    value_.reset();
}

// class Impl;

CellInterface::Value EmptyImpl::GetValue() const {
    return {};
}

std::string EmptyImpl::GetText() const {
    return {};
}

std::vector<Position> EmptyImpl::GetReferencedCells() const {
    return {};
}

CellInterface::Value TextImpl::GetValue() const {
    if (text_[0] == ESCAPE_SIGN){
        return text_.substr(1);
    }
    return text_;
}

std::string TextImpl::GetText() const {
    return text_;
}

std::vector<Position> TextImpl::GetReferencedCells() const {
    return {};
}

CellInterface::Value FormulaImpl::GetValue() const {
    const auto value = ast_ -> Evaluate(sheet_);
    if (std::holds_alternative<FormulaError>(value)){
        return std::get<FormulaError>(value);
    }
    return std::get<double>(value);
}

std::string FormulaImpl::GetText() const {
    return FORMULA_SIGN + ast_ -> GetExpression();
}

std::vector<Position> FormulaImpl::GetReferencedCells() const {
    return ast_ -> GetReferencedCells();
}
