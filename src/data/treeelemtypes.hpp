#ifndef TREEELEMTYPES_HPP
#define TREEELEMTYPES_HPP

/*
 * The possible kinds of data stored in our abstract data model
*/
enum struct [[maybe_unused]] eTreeElemDataType
{
    None,
    sprite,
    effectOffsets,
    palette,
    images,
    image,
    frames,
    frame,
    framepart,
    animSequences,
    animSequence,
    animTable,
    animGroup,
    animFrame,

    INVALID,
};

#endif // TREEELEMTYPES_HPP
