
# Arduino PO Beatbox
by Tim Schmidt

<!DOCTYPE html>
  <table>
    <tr>
      <td>
        <img src="box-complete.jpg" alt="Beatbox with Oscilloscope" />
      </td>
      <td>
        <img src="gear-complete.jpg" alt="Full Kit" />
      </td>
      <td>
        <img src="state-machine.jpg" alt="State Machine diagram" />
      </td>
    </tr>
  </table>
</html>


This project aims to address the challenge of daisy-chaining multiple Teenage Engineering Pocket Operators, which often results in dependent volume and signal levels from one unit to the next. To tackle this issue, the Arduino PO Beatbox offers a solution: it can send a synchronization pulse in parallel to all units and then mix them together, providing individual control over the signal output to the amplifier.

One significant contribution made during this project was the development of a chip driver for the ICM7218C LED Driver, which did not exist in the Arduino module library previously. The ICM7218C LED Driver is crucial for the functioning of this project, enabling synchronized visual feedback for the Pocket Operators.

## ICM7218C LED Driver Library
You can find the ICM7218C LED Driver library I produced here: [ICM7218C LED Driver Library](https://github.com/tttmmmsss/ICM7218C)

## Project Images
- ICM7218C Specification ![ICM7218C spec](ICM7218C%20spec.png)
- PEC12R Placement ![PEC12R Placement](PEC12R%20Placement.png)
- Daughterboard ![Daughterboard](daughterboard.png)
- Mainboard ![Mainboard](mainboard.png)
- Work in Progress 1 ![In Progress 1](in_progress_1.jpg)
- Work in Progress 2 ![In Progress 2](in_progress_2.jpg)

## Arduino Code
The Arduino sketch for the Arduino PO Beatbox can be found in the "beatbox_fullhardware.ino" file.

## Video Demonstrations
- Feature Demo: [Watch the Feature Demo](https://youtu.be/wUSr6W50LJI)
- Tuning Output Voltage: [Tuning Output Voltage Video](https://youtu.be/fswCqiL28x0)

---

Please refer to the [Google Photos Album](https://photos.app.goo.gl/SWdQEVwYZYm3CNe89) for additional media.
```

