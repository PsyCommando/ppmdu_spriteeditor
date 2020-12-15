#ifndef ANIMTATIONTABLELAYOUT_HPP
#define ANIMTATIONTABLELAYOUT_HPP
#include <QString>

class AnimTableModel;
//Tool for saving the layout of animation tables to xml and reading them
void ImportAnimTableLayout(AnimTableModel*       dst, const QString & filepath);
void ExportAnimTableLayout(const AnimTableModel* src, const QString & filepath);

#endif // ANIMTATIONTABLELAYOUT_HPP
