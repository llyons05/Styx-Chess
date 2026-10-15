#pragma once

#include "move.hpp"
#include "board.hpp"

#include <array>
#include <cstdint>
#include <vector>

const int MAX_MOVES = 218;

class Move_Generator {
public:
    std::vector<Move> movelist;
    BB64 destinations;

    Move_Generator(const Board& board) : movelist(), destinations(0), board(board), bbs(board.bbs) {
        // movelist.reserve(previous_movecount);
        if (board.stm() == WHITE)
            gen_moves<WHITE>();
        else
            gen_moves<BLACK>();
    }

    constexpr Move& operator[](U8 index) {
        assert(index < movelist.size());
        return movelist[index];
    }

    static bool is_attacked(const Board& board, eColor attackers, BB64 bb);

private:
    const Board& board;
    const Bitboards& bbs;

    template<eColor C>
    std::vector<Move>& gen_moves();

    void write_moves(ePieceType piece_from, ePieceType piece_to, eSquare from, BB64 to, eSpecialMoveFlags flag);

    template<eColor C>
    void gen_pawn_moves();

    void gen_bishop_moves();
    void gen_knight_moves();
    void gen_rook_moves();
    void gen_queen_moves();
    void gen_king_moves();

    template<Direction D>
    static BB64 valid_sliding_moves(BB64 pieces, const Board& board, eColor color);

    template <eColor C>
    static BB64 pawn_attacks(const Board& board);

public:
    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type        = Move;
        using difference_type   = std::ptrdiff_t;
        using pointer           = Move*;
        using reference         = Move&;

        iterator() = default;

        reference operator*() const {
            return owner->movelist[index];
        }

        pointer operator->() const {
            return &owner->movelist[index];
        }

        // Advance to the next highest-scoring move.
        iterator& operator++() {
            assert(owner != nullptr);
            assert(index < owner->movelist.size());

            const std::size_t next = index + 1;
            const std::size_t size = owner->movelist.size();

            if (next < size) {
                std::size_t best = next;

                for (std::size_t i = next + 1; i < size; ++i) {
                    if (owner->movelist[i].score >
                        owner->movelist[best].score) {
                        best = i;
                    }
                }

                if (best != next) {
                    std::swap(owner->movelist[next],
                              owner->movelist[best]);
                }
            }

            ++index;
            return *this;
        }

        iterator operator++(int) {
            iterator old = *this;
            ++(*this);
            return old;
        }

        bool operator==(const iterator& other) const {
            return owner == other.owner && index == other.index;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        friend class Move_Generator;

        Move_Generator* owner = nullptr;
        std::size_t index = 0;

        iterator(Move_Generator* owner, std::size_t index)
            : owner(owner), index(index) {}
    };

    iterator begin() {
        return iterator(this, 0);
    }

    iterator end() {
        return iterator(this, movelist.size());
    }
};


constexpr std::array<BB64, 64> init_knight_moves() {
    std::array<BB64, 64> result;
    for (int s = 0; s < NULL_SQUARE; s++) {
        BB64 knight = BB64(eSquare(s));
        BB64 valid_moves = knight.shift<NORTH, 2>().shift<EAST>();
        valid_moves |= knight.shift<NORTH, 2>().shift<WEST>();

        valid_moves |= knight.shift<SOUTH, 2>().shift<EAST>();
        valid_moves |= knight.shift<SOUTH, 2>().shift<WEST>();

        valid_moves |= knight.shift<EAST, 2>().shift<NORTH>();
        valid_moves |= knight.shift<EAST, 2>().shift<SOUTH>();
        
        valid_moves |= knight.shift<WEST, 2>().shift<NORTH>();
        valid_moves |= knight.shift<WEST, 2>().shift<SOUTH>();

        result[s] = valid_moves;
    }
    return result;
}

constexpr std::array<BB64, 64> init_king_moves() {
    std::array<BB64, 64> result;
    for (int s = 0; s < NULL_SQUARE; s++) {
        BB64 king = BB64(eSquare(s));
        BB64 valid_moves = king.shift<NORTH>();
        valid_moves |= king.shift<NORTHEAST>();
        valid_moves |= king.shift<EAST>();
        valid_moves |= king.shift<SOUTHEAST>();
        valid_moves |= king.shift<SOUTH>();
        valid_moves |= king.shift<SOUTHWEST>();
        valid_moves |= king.shift<WEST>();
        valid_moves |= king.shift<NORTHWEST>();
        result[s] = valid_moves;
    }
    return result;
}

constexpr std::array<BB64, 64> knight_moves = init_knight_moves();
constexpr std::array<BB64, 64> king_moves = init_king_moves();