#include "scenerenderer.hpp"

SceneRenderer::SceneRenderer(bool bshouldloop, QObject *parent)
    : QObject(parent), m_animsprite(nullptr), m_ticks(0), m_shouldLoop(bshouldloop), m_spr(nullptr), m_seqid(-1)
{
Reset();
}
