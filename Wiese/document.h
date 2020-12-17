#ifndef WIESE_DOCUMENT_H_
#define WIESE_DOCUMENT_H_

#include <cassert>
#include <list>
#include <string_view>
#include <vector>

namespace wiese {

class Piece {
 private:
  enum class Kind { kOriginal, kPlain, kLineBreak };

 public:
  static Piece MakeOriginal(int start, int end);
  static Piece MakePlain(int start, int end);
  static Piece MakeLineBreak();
  bool IsOriginal() const { return kind_ == Kind::kOriginal; }
  bool IsPlain() const { return kind_ == Kind::kPlain; }
  bool IsLineBreak() const { return kind_ == Kind::kLineBreak; }
  int GetCharCount() const {
    switch (kind_) {
      case Kind::kOriginal:
      case Kind::kPlain:
        return end_ - start_;
      case Kind::kLineBreak:
        return 1;
    }
    assert(false);
    return 0;
  }
  Piece SplitAt(int index) {
    Piece rest(*this);
    rest.end_ = end_;
    rest.start_ = end_ = start_ + index;
    return rest;
  }
  Piece Slice(int start, int end) const {
    Piece sub_piece(*this);
    sub_piece.start_ = start_ + start;
    sub_piece.end_ = start_ + end;
    return sub_piece;
  }
  int start() const {
    assert(IsOriginal() || IsPlain());
    return start_;
  }
  void set_start(int value) {
    assert(IsOriginal() || IsPlain());
    assert(value <= end_);
    start_ = value;
  }
  int end() const {
    assert(IsOriginal() || IsPlain());
    return end_;
  }
  void set_end(int value) {
    assert(IsOriginal() || IsPlain());
    assert(start_ <= value);
    end_ = value;
  }
  bool operator==(const Piece& rhs) const {
    return kind_ == rhs.kind_ && start_ == rhs.start_ && end_ == rhs.end_;
  }

 private:
  Piece(Kind kind) : kind_(kind) {}
  Kind kind_;
  int start_;
  int end_;
};

class LineIterator;

class Document {
 public:
  using PieceList = std::list<Piece>;
  using PieceListIterator = PieceList::const_iterator;

  Document(const wchar_t* original_text);
  Document(const Document&) = delete;
  Document& operator=(const Document&) = delete;

  void InsertCharBefore(wchar_t ch, int position);
  void InsertStringBefore(const wchar_t* string, int position);
  void InsertLineBreakBefore(int position);
  wchar_t EraseCharAt(int position);
  wchar_t EraseCharAt(int line, int offset);

  std::wstring GetText() const;
  int GetCharCount() const;
  wchar_t GetCharAt(int position) const;

  std::wstring_view GetCharsInPiece(const Piece& piece) const;
  PieceListIterator PieceIteratorBegin() const { return pieces_.begin(); }
  PieceListIterator PieceIteratorEnd() const { return pieces_.end(); }
  LineIterator LineBegin() const;

 private:
  Piece AddCharsToBuffer(const wchar_t* chars, int count);
  void InsertCharsBefore(const wchar_t* chars, int count, int position);
  wchar_t GetCharInPiece(const Piece& piece, int index) const;
  wchar_t EraseCharInFrontOf(PieceList::iterator it);

  PieceList pieces_;
  const std::vector<wchar_t> original_;
  std::vector<wchar_t> added_;
};

class LineIterator {
 public:
  LineIterator(Document::PieceListIterator it, Document::PieceListIterator end) : it_(it), end_(end) {}
  Document::PieceListIterator operator*() { return it_; }
  LineIterator& operator++() {
    assert(it_ != end_);
    do {
      ++it_;
      if (it_ == end_) return *this;
    } while (!it_->IsLineBreak());
    ++it_;
    return *this;
  }
 private:
  Document::PieceListIterator it_;
  Document::PieceListIterator end_;
};

inline LineIterator Document::LineBegin() const {
  return LineIterator(pieces_.begin(), pieces_.end());
}

}  // namespace wiese

#endif
