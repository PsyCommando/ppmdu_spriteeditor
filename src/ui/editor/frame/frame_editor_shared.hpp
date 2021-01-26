#ifndef FRAME_EDITOR_SHARED_HPP
#define FRAME_EDITOR_SHARED_HPP

//Operation modes for the frame editor
enum struct eEditorMode : int
{
    FrameParts,
    AttachmentPoints,

    NbModes [[maybe_unused]],
    Invalid [[maybe_unused]],
};

#endif // FRAME_EDITOR_SHARED_HPP
