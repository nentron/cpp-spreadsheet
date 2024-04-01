#pragma once

#include "cell.h"
#include "common.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>


class Sheet : public SheetInterface{
private:

    const std::unique_ptr<Cell> EmptyCell = std::make_unique<Cell>(*this);

    std::unordered_map<Position, std::unique_ptr<Cell>, PositionHash> table_;
    using DependedCells = std::unordered_set<Position, PositionHash>;
    std::unordered_map<
        Position, DependedCells,
        PositionHash> depended_cells_;
    int rows_ = 0;
    int cols_ = 0;
    void ReducePrintableSize();

    bool SuccessSet(std::unique_ptr<Cell>& cell, Position pos, std::string text);

    void InvalidateCache(const DependedCells& depended_cells);

    void RemoveOldDependedCells(Position cell, const std::vector<Position>& cells);

    void AddNewDependedCells(Position cell, const std::vector<Position>& cells);

    void CheckPosValidation(Position pos) const;
public:
    ~Sheet();

    Sheet() = default;

    void SetCell(Position pos, std::string text) override;
    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& out) const override;
    void PrintTexts(std::ostream& out) const override;
};
