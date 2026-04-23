const express = require('express');
const app = express();

app.use(express.json());

app.post('/update', (req, res) => {
  console.log(req.body);

  res.status(200).json(req.body);
});

app.listen(8080, () => {
  console.log('REST API server running on port 8080');
}); 