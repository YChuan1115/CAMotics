/******************************************************************************\

    CAMotics is an Open-Source simulation and CAM software.
    Copyright (C) 2011-2017 Joseph Coffland <joseph@cauldrondevelopment.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

\******************************************************************************/

#include "MachineLinearizer.h"

using namespace cb;
using namespace GCode;


void MachineLinearizer::arc(const cb::Vector3D &offset, double angle,
                            plane_t plane) {
  const char *axesNames;
  switch (plane) {
  case XY: axesNames = "XYZ"; break;
  case XZ: axesNames = "XZY"; break;
  case YZ: axesNames = "YZX"; break;
  default: THROWS("Invalid plane: " << plane);
  }

  unsigned char axes[3];
  for (unsigned i = 0; i < 3; i++)
    axes[i] = Axes::toIndex(axesNames[i]);

  Axes current = getPosition();

  // Initial point
  double x = current[axes[0]];
  double y = current[axes[1]];
  double z = current[axes[2]];

  // Center
  cb::Vector2D center(x + offset.x(), y + offset.y());

  // Start angle
  double startAngle =
    cb::Vector2D(-offset.x(), -offset.y()).angleBetween(cb::Vector2D(1, 0));

  // Radius
  double radius = cb::Vector2D(offset.x(), offset.y()).length();

  // Allowed error cannot be greater than arc radius
  double error = std::min(maxArcError, radius);
  double errorAngle = 2 * acos(1 - error / radius);

  // Error angle cannot be greater than 2Pi/3 because we need at least 3
  // segments in a full circle
  errorAngle = std::min(2 * M_PI / 3, errorAngle);

  // Segments
  unsigned segments = (unsigned)ceil(fabs(angle) / errorAngle);
  double deltaAngle = -angle / segments;
  double zDelta = offset.z() / segments;

  // TODO The estimated arc should straddle the actual arc.  This one

  // Create segments
  for (unsigned i = 0; i < segments; i++) {
    double currentAngle = startAngle + deltaAngle * (i + 1);

    x = center.x() + radius * cos(currentAngle);
    y = center.y() + radius * sin(currentAngle);
    z += zDelta;

    // Move
    current[axes[0]] = x;
    current[axes[1]] = y;
    current[axes[2]] = z;
    move(current, false);
  }
}
