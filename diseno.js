/*problema: hacer que el valor de angulo sea el que el usuario eligio
            sea constante y variar la intensidad leida desde el serialport del arduino
            
            Propuesta de solucion:
            Por lo tanto ... el angulo se pasara como variable a x pero tendra que tomar 
            para cada intensidad diferente y los valores del serialport que estaran midiendo 
            la intencidad se pasaran a value en cada medida.
            
            */

//donde x es el angulo y el value la intencidad para cada angulo.
//VALORES.


var grafica = document.getElementById("obtener")

chart = anychart.polar();

grafica.addEventListener('click', function(){


    fetch('http://192.168.1.95/DATA')
    //fetch('http://192.168.100.9/DATA')
    .then(res => {


        return res.json()


    }).then(json =>{
        

        console.log(json)

        var series = chart.marker( json )

        chart.bounds(0, 0, "100%", "100%");

        // set the maximum value of the x-scale
        chart.xScale().maximum(360);

        // set the container id
        chart.container("container");

        // initiate drawing the chart
        chart.draw();       
        
        
    }).catch(err => {
        console.log(err.message)
    })

})



